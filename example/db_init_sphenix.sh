#!/usr/bin/env bash

xpl insert example_tag_1 Beam     D0DXMagnets.dat -s 5
xpl insert example_tag_1 Beam     D0DXMagnets_rev1.dat -s 10
xpl insert example_tag_1 MagField sphenix3dbigmapxyz.root
xpl insert example_tag_1 CEMC     CEMCprof_Thresh30MeV.root -s 10
xpl insert example_tag_1 CEMC     CEMCprof_Thresh30MeV_rev1.root -s 20
xpl insert example_tag_1 ZDC      towerMap_ZDC.txt
xpl insert example_tag_1 CEMCGeo   cemc_geoparams-0-0-4294967295-1536789215.xml
