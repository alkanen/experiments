from flask import Flask, request, send_file

from jobs_handler import JobsHandler


class RestApi:
    app = Flask(__name__)

    def __init__(self, config):
        self.config = config
        jl = config["jobs"]
        self.jobs = {
            job: JobsHandler(
                folder=jl[job]["folder"],
                num_samples=100,
                timeout=5 * 60,
                camera=jl[job]["camera"],
                image=jl[job]["image"],
                textures=jl[job]["textures"],
                materials=jl[job]["materials"],
                objects=jl[job]["objects"],
            )
            for job in jl
        }

    def run(self):
        @RestApi.app.route("/jobs", methods=["GET"])
        def list_jobs():
            joblist = {"jobs": [x for x in self.config["jobs"]]}
            return joblist

        @RestApi.app.route("/job/<jobname>", methods=["GET"])
        def get_job(jobname):
            return self.jobs[jobname].get_job("test_client")

        @RestApi.app.route("/job/<jobname>", methods=["POST"])
        def post_job(jobname):
            reference = request.json["reference"]
            data = request.json["data"]
            return self.jobs[jobname].report_job(reference, data)

        @RestApi.app.route("/job/<jobname>/scene/<scene_hash>", methods=["GET"])
        def get_config(jobname, scene_hash):
            return self.jobs[jobname].get_scene(scene_hash)

        @RestApi.app.route("/status/<jobname>", methods=["GET"])
        def get_status(jobname):
            return self.jobs[jobname].get_status()

        @RestApi.app.route("/image/<jobname>", methods=["GET"])
        def get_image(jobname):
            print(f"image/{jobname}")
            tmp_file = self.jobs[jobname].get_image()
            return send_file(
                tmp_file,
                # as_attachment=True,
                # filename=f"{jobname}.png",
                mimetype="image/png",
            )

        @RestApi.app.route("/heatmap/<jobname>", methods=["GET"])
        def get_heatmap(jobname):
            print(f"heatmap/{jobname}")
            tmp_file = self.jobs[jobname].get_heatmap()
            return send_file(
                tmp_file,
                # as_attachment=True,
                # filename=f"{jobname}_heatmap.png",
                mimetype="image/png",
            )

        RestApi.app.run(
            host=self.config["host"],
            port=self.config["port"],
        )
