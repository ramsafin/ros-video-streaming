#!/usr/bin/env python3
# encoding: utf-8

import os
import re
import json
from typing import Final, List
from dataclasses import dataclass, asdict


V4L2_HEADER_FILEPATH: Final[os.PathLike]= "/usr/include/linux/videodev2.h"

V4L2_PIX_FMT_PATTERN: Final[str] = re.compile(
    r'#define\s+(V4L2_PIX_FMT_\w+)\s+v4l2_fourcc\(\s*\'(.)\',\s*\'(.)\',\s*\'(.)\',\s*\'(.)\'\s*\)'
)


@dataclass
class PixelFormat:
  name: str
  fourcc: str
  value: int


def parse_v4l2_formats() -> List[PixelFormat]:
    formats: List[PixelFormat] = []

    with open(V4L2_HEADER_FILEPATH, 'r') as header_file:
        for line in header_file:
            match = V4L2_PIX_FMT_PATTERN.search(line)

            if match:
                name, c1, c2, c3, c4 = match.groups()
                fourcc_str = ''.join([c1, c2, c3, c4])
                val = (ord(c1)) | (ord(c2) << 8) | (ord(c3) << 16) | (ord(c4) << 24)
                formats.append(PixelFormat(name.strip(), fourcc_str.strip(), val))

    return formats


def generate_header(formats: List[PixelFormat], output_path: os.PathLike) -> None:
    with open(output_path, "w") as out:
        out.write("#pragma once\n\n")
        out.write("#include <linux/videodev2.h>\n\n")
        out.write("#include <cstdint>\n")
        out.write("#include <optional>\n")
        out.write("#include <string_view>\n\n")

        out.write("namespace lirs::formats {\n\n")

        # Enum declaration
        out.write("enum class PixelFormat : uint32_t {\n")
        for fmt in formats:
            out.write(f"  V4L2_{fmt.fourcc} = {fmt.name},\n")
        out.write("};\n\n")

        # format2str function
        out.write("inline constexpr std::string_view format2str(PixelFormat fmt) {\n")
        out.write("  switch (fmt) {\n")
        for fmt in formats:
            out.write(f'    case PixelFormat::V4L2_{fmt.fourcc}: return "{fmt.name}";\n')
        out.write("    default: return \"UNKNOWN\";\n")
        out.write("  }\n")
        out.write("}\n\n")

        # fourcc2format function
        out.write("inline constexpr std::optional<PixelFormat> fourcc2format(uint32_t fourcc) {\n")
        out.write("  switch (fourcc) {\n")
        for fmt in formats:
            out.write(f"    case {fmt.name}: return PixelFormat::V4L2_{fmt.fourcc};\n")
        out.write("    default: return std::nullopt;\n")
        out.write("  }\n")
        out.write("}\n\n")

        out.write("} //  namespace lirs::formats\n")


if __name__ == "__main__":
    generate_header(parse_v4l2_formats(), "formats.hpp")
