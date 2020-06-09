import datetime
from urllib.parse import urlunparse
import uuid
from xml.etree.ElementTree import Element, SubElement


class Itunes:
    EXPLICIT = "Yes"
    NOT_EXPLICIT = "No"

    def __init__(
        self,
        title,
        description,
        hostname,
        author="",
        summary="",
        image="",
        categories=None,
        owner_name="",
        owner_email="",
        explicit=None,
        schema="https",
        port=None,
        root="",
        link="",
    ):
        self.title = title
        self.description = description
        self.link = link
        self.categories = categories if categories else []
        self.image = image

        # Network stuff
        self.hostname = hostname
        self.port = port
        self.root = root
        self.schema = schema

        # Itunes fields
        self.author = author
        self.summary = summary
        self.owner_name = owner_name
        self.owner_email = owner_email
        if explicit is None:
            self.explicit = Itunes.NOT_EXPLICIT
        else:
            self.explicit = explicit

        self.namespace = uuid.uuid5(uuid.NAMESPACE_DNS, self.hostname)

        self.items = []

    @property
    def base_url(self):
        return urlunparse(
            (
                self.schema,
                "%s%s" % (self.hostname, ":%d" % self.port if self.port else ""),
                f"{self.root}/",
                None,
                None,
                None,
            )
        )

    def add_item(self, item):
        self.items.append(item)

    def xml(self):
        rss = Element(
            "rss",
            {
                "xmlns:itunes": "http://www.itunes.com/dtds/podcast-1.0.dtd",
                "version": "2.0",
            },
        )

        channel = SubElement(rss, "channel")
        title = SubElement(channel, "title")
        title.text = self.title

        description = SubElement(channel, "description")
        description.text = self.description

        link = SubElement(channel, "link")
        link.text = self.link

        language = SubElement(channel, "language")
        language.text = "en-us"

        copyright = SubElement(channel, "copyright")
        copyright.text = "Copyright 2016-%4d" % datetime.datetime.now().year

        last_build_date = SubElement(channel, "lastBuildDate")
        last_build_date.text = datetime.datetime.now().strftime(
            "%a, %d %b %Y %H:%m:%S +0000"
        )
        pub_date = SubElement(channel, "pubDate")
        pub_date.text = datetime.datetime.now().strftime("%a, %d %b %Y %H:%m:%S +0000")

        docs = SubElement(channel, "docs")
        docs.text = "http://blogs.law.harvard.edu/tech/rss"

        webmaster = SubElement(channel, "webMaster")
        webmaster.text = ""

        ttl = SubElement(channel, "ttl")
        ttl.text = "60"

        # Itunes section
        author = SubElement(channel, "itunes:author")
        author.text = self.author

        summary = SubElement(channel, "itunes:summary")
        summary.text = self.summary

        owner = SubElement(channel, "itunes:owner")

        owner_name = SubElement(owner, "itunes:name")
        owner_name.text = self.owner_name

        owner_email = SubElement(owner, "itunes:email")
        owner_email.text = self.owner_email

        explicit = SubElement(channel, "itunes:explicit")
        explicit.text = self.explicit

        # Handle absolute and relative paths here
        SubElement(channel, "itunes:image", {"href": self.image})

        for category in self.categories:
            SubElement(channel, "itunes:category", {"text": category})

        for item in self.items:
            item.xml(channel)

        return rss
