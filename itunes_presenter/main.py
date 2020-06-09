from item import Item
from itunes import Itunes
from xml.etree.ElementTree import tostring
from xml.dom import minidom


def main():
    itunes = Itunes(
        title="Bram Stoker's Dracula",
        description="The classic story about the world's most famous vampire.",
        hostname="alkanen.no-ip.biz",
        schema="https",
    )

    itunes.add_item(
        Item.from_audio(
            parent=itunes,
            title="Dracula, 1 / 62",
            path="books/Audio Books/Bram Stoker/Dracula (48k)/Bram Stoker - Dracula 01_62.mp3",
            description="Bram Stoker's Dracula, part 1 of 62",
            explicit="No",
        )
    )

    xml = itunes.xml()
    xml_raw = tostring(xml, "utf-8")
    reparsed = minidom.parseString(xml_raw)
    print(reparsed.toprettyxml(indent="  ", encoding="utf-8").decode())


if __name__ == "__main__":
    main()
