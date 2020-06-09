import datetime
import os
import subprocess
import urllib
import uuid
from xml.etree.ElementTree import SubElement


class Item:
    def __init__(
        self,
        parent,
        title,
        path,
        description="",
        category="Podcasts",
        author="",
        explicit="No",
    ):
        self.parent = parent
        self.title = title
        self.path = path
        self.description = description
        self.category = category
        self.author = author
        self.explicit = explicit

    @classmethod
    def from_video(
        cls,
        parent,
        title,
        path,
        description="",
        category="",
        author="",
        explicit="No",
        subtitle="",
    ):
        item = cls(
            parent=parent,
            title=title,
            path=path,
            description=description,
            category=category,
            author=author,
            explicit=explicit,
        )
        item.type = "video/mpeg"
        item.subtitle = subtitle

        # ffmpeg -i "2010-01-13 14-40-08 pad MN_db_0310.avi" 2>&1 | grep "Duration" | cut -d ' ' -f 4 | sed s/,//

        return item

    @classmethod
    def from_audio(
        cls, parent, title, path, description="", category="", author="", explicit="No",
    ):
        item = cls(
            parent=parent,
            title=title,
            path=path,
            description=description,
            category=category,
            author=author,
            explicit=explicit,
        )
        item.type = "audio/mpeg"
        item._duration = None

        return item

    @property
    def duration(self):
        if self._duration is None:
            if self.type == "audio/mpeg":
                output = subprocess.check_output(
                    ["mp3info", "-p", "%S", self.path,], encoding="utf-8",
                )
                seconds = int(output)
                self._duration = "%d:%02d:%02d" % (
                    seconds // 3600,
                    (seconds // 60) % 60,
                    seconds % 60,
                )

            elif self.type == "video/mpeg":
                pass

            else:
                raise ValueError(f"Invalid mime type: {self.type}")

        return self._duration

    @property
    def url(self):
        return self.parent.base_url + urllib.parse.quote(self.path)

    def xml(self, parent_node):
        item = SubElement(parent_node, "item")

        title = SubElement(item, "title")
        title.text = self.title

        SubElement(item, "link")

        guid = SubElement(item, "guid")
        guid.text = str(uuid.uuid5(self.parent.namespace, self.path))

        description = SubElement(item, "description")
        description.text = self.description

        SubElement(
            item,
            "enclosure",
            {
                "url": self.url,
                "length": str(os.stat(self.path).st_size),
                "type": self.type,
            },
        )

        category = SubElement(item, "category")
        category.text = self.category

        pub_date = SubElement(item, "pubDate")
        pub_date.text = datetime.datetime.now().strftime("%a, %d %b %Y %H:%m:%S +0000")

        itunes_author = SubElement(item, "itunes:author")
        itunes_author.text = self.author
        itunes_explicit = SubElement(item, "itunes:explicit")
        itunes_explicit.text = self.explicit
        if self.type.startswith("video"):
            itunes_subtitle = SubElement(item, "itunes:subtitle")
            itunes_subtitle.text = self.subtitle
        SubElement(item, "itunes:summary")
        if self.duration is not None:
            itunes_duration = SubElement(item, "itunes:duration")
            itunes_duration.text = self.duration

        SubElement(item, "itunes:keywords")

        return item
