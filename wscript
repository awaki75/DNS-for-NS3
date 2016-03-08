# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('dns', ['core', 'network'])
    module.source = [
        'model/dns.cc',
        'model/dns-header.cc',
				'model/bind-server.cc',
        'helper/dns-helper.cc',
        ]

#    module_test = bld.create_ns3_module_test_library('dns')
#    module_test.source = [
#        'test/dns-test-suite.cc',
#        ]

    headers = bld(features='ns3header')
    headers.module = 'dns'
    headers.source = [
        'model/dns.h',
        'model/dns-header.h',
				'model/bind-server.h',        
        'helper/dns-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

