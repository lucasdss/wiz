import os
import shutil
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    component_dir = os.path.dirname(os.path.realpath(__file__))
    src_file = os.path.join(component_dir, "wiz_udp.h")
    dest_file = os.path.join(CORE.build_path, "src", "wiz_udp.h")
    shutil.copyfile(src_file, dest_file)
    cg.add_global(cg.RawStatement('#include "wiz_udp.h"'))
