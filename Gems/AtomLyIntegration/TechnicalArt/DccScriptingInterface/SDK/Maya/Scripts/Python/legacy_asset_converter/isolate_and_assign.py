# coding:utf-8
# !/usr/bin/python
#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
# File Description:
# This is a temporary workaround script for previewing Stingray materials applied to FBX files in material conversion.
# The reason this is needed is because of a bug found in automating Stingray material assignmens
# -------------------------------------------------------------------------


import pymel.core as pm
import maya.mel as mel
import constants
import random
import os


def get_materials_in_scene():
    for shading_engine in pm.ls(type=pm.nt.ShadingEngine):
        if len(shading_engine):
            for material in shading_engine.surfaceShader.listConnections():
                yield material


def get_file_textures(material_name):
    try:
        file_textures = []
        material_files = [x for x in pm.listConnections(material_name, plugs=1, source=1) if x.startswith('file')]
        for file_name in material_files:
            try:
                file_textures.append(pm.getAttr('{}.fileTextureName'.format(file_name.split('.')[0])))
            except pm.MayaAttributeError:
                pass
        return file_textures
    except AttributeError:
        return None


def get_search_string(texture_name):
    """
    Creates a "base" string derived from the passed texture name for finding associated files
    :param texture_name: The texture name from which to extract the search string
    :return:
    """
    filename = os.path.basename(texture_name)
    base = filename.split('.')[0]
    basename_list = base.split('_')
    search_string = '_'.join(basename_list[:-1])
    return search_string


def get_texture_type(search_string, file_name):
    """
    Gets the type of PBR texture for assignments based on the texture filename
    :param search_string: The convention basename for file texture
    :param file_name: This is a filename extracted from the mtl file- used purely as a template for newly created PBR textures
    :return:
    """
    texture_types = ['BaseColor', 'Metallic', 'Normal', 'Emissive', 'Roughness']
    target_string = file_name.replace(search_string, '')
    base = target_string.split('.')[0]
    if base[1:] in texture_types:
        return base[1:]
    return None


def get_output_file_size(file_path):
    """
    When creating texture masks for further processing, this function checks output filesize to determine if there are
    any UVs present for the object it's created for. There are many assets without UVS or with UVs that entirely
    encapsulate the 0 to 1 UV space- and they come out in the size fiting between the range below. Without any content
    there is no need for the generated mask- so it is summarily removed
    :param file_path: Path to the vertex map masks generated by object UVs
    :return:
    """
    size = os.stat(file_path).st_size
    if (constants.EMPTY_IMAGE_LOW < size < constants.EMPTY_IMAGE_HIGH) == True:
        os.remove(file_path)


def attach_texture(texture_type, material, shading_group_name, file_path):
    material_info = {
        'BaseColor': 'color',
        'Metallic': 'metallic',
        'Normal': 'normal',
        'Emissive': 'emissive',
        'Roughness': 'roughness'
    }

    file_count = len(pm.ls(type='file')) + 1
    texture_file = 'file{}'.format(file_count)
    file_node = pm.shadingNode('file', asTexture=True, name=texture_file)
    pm.setAttr('{}.fileTextureName'.format(texture_file), file_path, type="string")
    pm.setAttr('{}.use_{}_map'.format(material, material_info[texture_type]), True)
    pm.connectAttr('{}.outColor'.format(texture_file), '{}.TEX_{}_map'.format(material, material_info[texture_type]),
                   force=True)


def combine_meshes():
    """
    Combines geometry based on material assignment. This function was created before the incorporation of .assetinfo
    files while processing. I intend to revamp the system, as this practice was found to be problematic when LOD meshes
    were found in files containing the same materials. Leaving this in for now until a better system can be created as
    this still helps in many cases to produce a mesh for preview
    :return:
    """
    pm.showHidden(all=True)
    pm.select(pm.ls(geometry=True))
    selected_geo = pm.selected()
    print(selected_geo)
    if len(selected_geo) > 1:
        combined_mesh = pm.polyUnite(selected_geo)
        pm.delete(ch=True)
        return combined_mesh[0]
    else:
        return selected_geo


def create_material_masks(target_mesh, material_list, base_directory):
    """
    Fuction for generating vertex map masks of existing UVs. If metallic maps needed to be constructed by hand this
    made mask assignments in Substance Designer a quicker process. There might still be a need for this process for
    other things so I'm leaving this in here for now- eventually it might land in a Maya utility class.
    :param target_mesh: The mesh to generate maps from
    :param material_list: The list of materials needing masks generated for
    :param base_directory: The directory to save vertex map masks to
    :return:
    """
    container = separate_applied_materials(target_mesh)
    children = pm.listRelatives(container, children=True)

    for item in children:
        if not item.endswith('_pbr_GEO'):
            pm.delete(item)
        else:
            pm.hide(item)

    print('\n::::::::::::::::::::::::::::::::')
    print('Bake material masks')
    print('\n::::::::::::::::::::::::::::::::')
    print('Children: {}'.format(children))
    print('Material List: {}'.format(material_list))

    for material in material_list:
        print('Material: {}'.format(material))
        separated_mesh_object = '{}_GEO'.format(material)
        if separated_mesh_object in children:
            pm.showHidden(separated_mesh_object)
            bake_vertex_colors(separated_mesh_object, material, base_directory)
            cleanup_object(separated_mesh_object, material)
        print('\n')

    pm.setToolTo('selectSuperContext')
    pm.select(clear=True)


def cleanup_object(target_mesh, material):
    """
    Vertex masks are created using vertex colors, but once the maps have been generated the vertex colors should be
    removed. This function handles the cleanup process.
    :param target_mesh:
    :param material:
    :return:
    """
    pm.select(target_mesh)
    pm.polyColorPerVertex(rem=True)
    pm.delete(ch=True)
    pm.hyperShade(assign=material)
    pm.hide()
    pm.select(clear=True)


def bake_vertex_colors(target_mesh, material, base_directory):
    file_path = get_file_path(material, base_directory)
    print('Baking verts for: {}    Path: {}'.format(target_mesh, file_path))
    pm.select(target_mesh)
    pm.hyperShade(assign='lambert1')
    mel.eval('artAttrColorPerVertexToolScript 4;')
    pm.polyOptions(colorMaterialChannel='DIFFUSE')
    pm.polyColorPerVertex(r=1, g=1, b=1, a=1, cdo=True)
    pm.select(clear=True)

    try:
        pm.select(target_mesh, r=True)
        mel.eval('artAttrColorPerVertexToolScript 4;')
        context = pm.currentCtx()
        pm.artAttrPaintVertexCtx(context, e=True, esf=file_path, fsx=4096, fsy=4096)
        get_output_file_size(file_path)
    except RuntimeError as e:
        print('+++++++++++++++++')
        print('Bake Failed: {}'.format(e))
        print('+++++++++++++++++')


def get_file_path(material, base_directory):
    texture_list = get_file_textures(material)
    for item in texture_list:
        search_string = get_search_string(item)
        target_file = os.path.join(base_directory, '{}_Metallic.tif'.format(search_string))
        if os.path.isfile(target_file):
            return target_file
    return None


def separate_applied_materials(target_object):
    target_object = target_object[0] if isinstance(target_object, list) else target_object
    try:
        shading_groups = list(set(pm.listConnections(target_object.getShape(), type='shadingEngine')))
    except AttributeError:
        shading_groups = list(set(pm.listConnections(target_object, type='shadingEngine')))

    materials_container = pm.group(empty=1, n='separated_materials')
    for count, sg in enumerate(shading_groups):
        material = pm.ls(pm.listConnections(sg), materials=True)[0]
        target_material = pm.listConnections(sg + '.surfaceShader')
        clone = '{}_GEO'.format(target_material[0])
        pm.duplicate(target_object, n=clone)
        pm.parent(clone, 'separated_materials')
        material = pm.listConnections(sg, s=True, d=False)
        pm.select(clone)
        pm.runtime.ConvertSelectionToFaces()
        temp_set = str(pm.sets())
        temp_grps = pm.listConnections(material[0], type='shadingEngine')
        pm.select(pm.sets(temp_grps[0], int=temp_set))
        pm.runtime.InvertSelection()
        pm.delete()

    pm.delete(target_object)
    return materials_container


def run():
    """
    The entry function of the script. This finds existing materials on FBX objects, converts to Stingray materials,
    and assigns corresponding textures.
    :return:
    """
    file_name = pm.sceneName()
    base_directory = os.path.dirname(pm.sceneName())
    print('Base Directory: {}'.format(base_directory))
    textured_materials = []
    for item in get_materials_in_scene():
        if str(type(item)) == "<class 'pymel.core.nodetypes.StingrayPBS'>":
            print('Material--> {}'.format(item))
            shading_group = pm.listConnections(item, type="shadingEngine")[0]
            file_textures = get_file_textures(item[:-4])
            if file_textures:
                print('FileTexture: {}'.format(file_textures))
                textured_materials.append(item)
                if len(file_textures) > 1:
                    print('Multiple textures found :::::::::::: {}'.format(file_textures))
                    print('Scene: {}   Material: {}'.format(file_name, item))
                else:
                    search_string = get_search_string(file_textures[0])
                    if search_string:
                        print('Search String: {}'.format(search_string))
                        print('Found these related textures:')
                        for target_file in os.listdir(base_directory):
                            if target_file.startswith(search_string):
                                print('-->>>>>> {}'.format(target_file))
                                texture_type = get_texture_type(search_string, target_file)
                                if texture_type:
                                    textured_materials.append(item)
                                    file_path = os.path.join(base_directory, target_file)
                                    attach_texture(texture_type, item, shading_group, file_path)

            print('')

    target_mesh = combine_meshes()
    textured_materials = list(set(textured_materials))
    create_material_masks(target_mesh, textured_materials, base_directory)
    pm.saveFile(force=True)
