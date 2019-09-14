from cybagoa import PyLayout, PyOALayoutLibrary

layout = PyLayout('utf-8')
layout.add_rect(('M1', 'drawing'), [[0.0, 0.0], [0.2, 0.1]],
                arr_nx=3, arr_ny=2, arr_spx=0.25, arr_spy=0.2)

layout.add_pin('foo', 'foo1', 'foo:', ('M2', 'pin'),
               [[0.5, 0.5], [0.7, 0.6]])

layout.add_via('M2_M1', (0.6, 0.6), 'R0', 2, 3, 0.06, 0.06,
               [0.04, 0.05, 0.0, 0.0], [0.0, 0.0, 0.06, 0.05],
               arr_nx=4, arr_spx=0.5)

with PyOALayoutLibrary('./cds.lib', 'AAAFOO', 'utf-8') as lib:
    lib.add_purpose('pin', 251)
    lib.create_layout('testpyoa', 'layout', layout)
