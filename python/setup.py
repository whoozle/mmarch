import setuptools

with open("../README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="mmarch-pack",
    version="1.0.0",
    author="Vladimir Menshakov",
    author_email="vladimir.menshakov@gmail.com",
    description="Archiving utility for mmarch project, mmap-friendly archive",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/whoozle/mmarch",
    packages=setuptools.find_packages(),
    scripts=['bin/mmarch-pack'],
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
)
