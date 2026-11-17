import os
import esphome.codegen as cg
import esphome.config_validation as cv

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    component_dir = os.path.dirname(os.path.realpath(__file__))
    cg.add_build_flag(f"-I{component_dir}")
    cg.add_global(cg.RawStatement('#include "wiz_udp.h"'))
