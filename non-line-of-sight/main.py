from math import ceil, floor, sqrt
import os

try:
    from numba import jit, prange
except ImportError:

    def jit(
        signature_or_function=None,
        locals={},
        target="cpu",
        cache=False,
        pipeline_class=None,
        boundscheck=False,
        **options
    ):
        def decorator(func):
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs)

            return wrapper

        return decorator

    prange = range


import numpy as np
from PIL import Image
from tqdm import tqdm

from loadmat import loadmat


# np.finfo(np.float64).eps
# eps = 2.220446049250313e-16
# np.finfo(np.float32).eps
eps = 1.1920929e-07

# Speed of light in m/s
c = 299792458


def calibrate(tau, calibration, resolution_t=32e-12):
    h, w = calibration.shape
    for y in tqdm(range(h)):
        for x in range(w):
            delay = floor(calibration[y, x] * 1e-12 / resolution_t)
            tau[..., y, x] = np.roll(tau[..., y, x], -delay, 0)
    return tau


@jit(nopython=True, parallel=True)
def downsample_and_crop(tau, resolution_xy, crop_t):
    def downsample(tau):
        T, h, w = tau.shape

        x_size = (w + 1) // 2
        y_size = (h + 1) // 2

        for y in prange(y_size):
            for x in prange(x_size):
                tmp_x = 2 * x
                tmp_y = 2 * y

                tau[..., y, x] = 0.25 * (
                    tau[..., min(tmp_y, h - 1), min(tmp_x, w - 1)]
                    + tau[..., min(tmp_y + 1, h - 1), min(tmp_x, w - 1)]
                    + tau[..., min(tmp_y + 1, h - 1), min(tmp_x + 1, w - 1)]
                    + tau[..., min(tmp_y, h - 1), min(tmp_x + 1, w - 1)]
                )

        return tau[..., 0:y_size, 0:x_size]

    T, h, w = tau.shape
    tau = tau[0 : min(T, crop_t), 0:h, 0:w]

    while tau.shape[2] > resolution_xy and tau.shape[1] > resolution_xy:
        tau = downsample(tau)

    return tau


@jit(nopython=True)
def stolt_linear_interpolation_t(A, t, y, x):
    t_low = floor(t)
    t_high = ceil(t)

    if t_low >= 0 and t_low < A.shape[0]:
        low = A[t_low, y, x]
    else:
        low = 0

    if t_high >= 0 and t_high < A.shape[0]:
        high = A[t_high, y, x]
    else:
        high = 0

    return low * (t_high - t) + high * (t - t_low)


@jit(nopython=True)
def stolt_interpolation_sub(A, t, y, x, resolution_t, physical_y, physical_x):
    T, h, w = A.shape

    i = 2 * x / w - 1
    j = 2 * y / h - 1
    k = 2 * t / T - 1

    if k <= 0:
        return 0

    i *= 0.5 * w * c * resolution_t / (2 * physical_x)
    j *= 0.5 * h * c * resolution_t / (2 * physical_y)

    length = sqrt(i ** 2 + j ** 2 + k ** 2)
    f = (length + 1) * T / 2
    S = max(c / 2 * k / max(length, eps), 0)

    return S * stolt_linear_interpolation_t(A, f, y, x)


@jit(nopython=True, parallel=True)
def stolt_interpolation(A, resolution_t, physical_y, physical_x):
    phi = np.zeros_like(A)
    for K_t in prange(phi.shape[0]):
        for K_y in prange(phi.shape[1]):
            for K_x in prange(phi.shape[2]):
                phi[K_t, K_y, K_x] = stolt_interpolation_sub(
                    A, K_t, K_y, K_x, resolution_t, physical_y, physical_x,
                )
    return phi


# @jit(forceobj=True, parallel=True)
def perform_fft(tau, shape):
    T, h, w = shape

    print("Convert to amplitude")
    psi = np.zeros((T * 2) * (h * 2) * (w * 2), dtype=np.float32).reshape(
        (T * 2, h * 2, w * 2)
    )
    psi[0:T, 0:h, 0:w] = np.sqrt(tau)
    tau = None

    print("Perform FFT")
    phi_bar_unshifted = np.fft.fftn(psi.astype(np.float32)).astype(np.complex64)
    psi = None

    print("Shift data")
    phi_bar = np.fft.fftshift(phi_bar_unshifted.astype(np.complex64)).astype(
        np.complex64
    )
    phi_bar_unshifted = None

    return phi_bar


# @jit(forceobj=True, parallel=True)
def unperform_fft(phi, shape):
    T, h, w = shape

    print("Unshift data")
    phi_unshifted = np.fft.ifftshift(phi)
    phi = None

    print("Reverse FFT")
    psi_new = np.fft.ifftn(phi_unshifted)
    phi_unshifted = None

    print("Amplitude -> Intensity")
    tau_new = np.zeros(T * h * w, dtype=np.float32).reshape(shape)
    tau_new = np.absolute(psi_new[0:T, 0:h, 0:w])
    tau_new = np.square(tau_new)

    return tau_new


def load_data(folder, dataset):
    if not folder.endswith("/"):
        folder = folder + "/"
    datafilename = os.path.join(folder, dataset)

    print("Loading data set, this may take a while")
    data = loadmat(datafilename)
    print("Loaded data with shape %s" % (data["meas"].shape,))

    return data["meas"]


def load_calib(folder):
    if not folder.endswith("/"):
        folder = folder + "/"
    calibfilename = os.path.join(folder, "tof.mat")

    print("Loading calibration data")
    calib = loadmat(calibfilename)

    return calib["tofgrid"]


def main():
    datafolder = "./statue/"

    data = load_data(datafolder, "meas_180min.mat")
    delays = load_calib(datafolder)

    T, h, w = data.shape
    assert (
        h == delays.shape[0] and w == delays.shape[1]
    ), "Delay calibration values do not match data"

    # Calibration data used to shift the data around so all spacial
    # points within one temporal point arrived at the SCREEN at the
    # same time.  Original data shows when spacial points arrived
    # at the LENS at the same time so screen points further away from
    # the camera will be part of a later temporal plane.
    print("Calibrate data set")
    tau = calibrate(data, delays)
    delays = None
    data = None

    print("Resample input data")
    tau = downsample_and_crop(tau, 256, 512)
    T, h, w = tau.shape

    # Convert intensity data to amplitude, pad to double size with
    # zeroes, perform FFT transform avd finally shift down to center
    # around zero.
    phi_bar = perform_fft(tau, (T, h, w))

    print("Interpolate data")
    phi = stolt_interpolation(phi_bar, resolution_t=32e-12, physical_y=2, physical_x=2,)
    phi_bar = None

    tau_new = unperform_fft(phi, (T, h, w))

    print("Deal with NaNs")
    np.nan_to_num(tau_new, copy=False, posinf=255)

    print("Save image")
    plane = np.max(tau_new, axis=0) / np.max(tau_new)
    plane = np.rot90(plane, 3)
    im = Image.fromarray((np.fliplr(plane) * 255).astype(np.uint8))
    im.convert("L").save("images/test.png")


if __name__ == "__main__":
    main()
