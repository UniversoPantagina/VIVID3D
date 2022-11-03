#include "TypeCasting.h"
#include "VoronoiVolume.h"
#include "ModelComponent.h"
#include "Mesh.h"
#include "Line.h"
#include "PointCloud.h"
#include "Shapes.h"
#include "Model.h"
#include "Animation.h"
#include "StopMotionAnimation.h"
#include <vector>
#include <array>
#include "Vivify.h"
#include "BlobData.h"
/*#include "./ImportAndExport/FBXImportExport.h"*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#define PYBIND11_DETAILED_ERROR_MESSAGES

using namespace vivid;
using namespace std;

namespace py = pybind11;
using namespace py::literals;
constexpr auto DEFAULT_COLOR_SHAPES = "lightgray";

py::array_t<double> make_cpoint(const double aValue) {
    py::array_t<double> array (3);
    py::buffer_info buffer = array.request();
    auto ptr = static_cast<double *>(buffer.ptr);
    for (int i = 0; i < 3; i++) {
        ptr[i] = aValue;
    }
    return array;
}

py::array_t<unsigned char> make_color_t(const unsigned char aValue) {
    py::array_t<unsigned char> array (3);
    py::buffer_info buffer = array.request();
    auto ptr = static_cast<unsigned char *>(buffer.ptr);
    for (int i = 0; i < 3; i++) {
        ptr[i] = aValue;
    }
    return array;
}


PYBIND11_MODULE(_core, m) {
    m.doc() = R"mydelimitter(
Core Module
===========

This Module contains the VIVID3D primary classes and functions for creating and exporting 3D models and animations.

)mydelimitter";

    py::class_<CBlobData>(m, "BlobData", R"mydelimiter(
Blob files containing exported file data.

Attributes
----------
n_files : number
    Number of files held
names : list[str]
    List of file names
files : list[bytes]
    List of files held

        )mydelimiter")
        .def_readonly("n_files", &CBlobData::mNumFiles, "Number of files held")
        .def_readonly("names", &CBlobData::mNames, "List of file names held")
        .def_property_readonly("files", [](const CBlobData& arSelf) {
            std::vector<py::bytes> bytes;
            for (const auto& file : arSelf.mFiles) {
                bytes.push_back(py::bytes(file));
            }
            return bytes;
        }, "List of files held. All files are binary encoded arrays");

    py::class_<CPoint>(m, "Point")
        .def(py::init<const CPoint &>(),
                "copy constructor for Point",
                py::arg("point"))
        .def(py::init<coord_t, coord_t, coord_t>(),
             "Constructor for point",
             py::arg("x"), py::arg("y"), py::arg("z"))
        .def(py::init<std::vector<double>>(),
                 "Constructor for point",
                 py::arg("3d_vector"));

    py::class_<CMaterial>(m, "Material", R"mydelimiter(
Simplified PBR Material for physically based rendering of Meshes

Attributes
----------
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
shininess : (0.0-1.0), default: 0.1
    Shininess affects the reflectivity of a mesh.
emission_strength : (0.0-1.0), default: 0.0
    Float in the range of 0.0 - 1.0 indicating the self-illumination strength of the material.
emission_color : str, default: "black"
    The color of self-illumination. See https://htmlcolorcodes.com/color-names/ for list of all available colors."
name : str, default: "defaultMaterial"
    Optional name of the object, not required to be unique.

    )mydelimiter")
        .def(py::init<normal_float, normal_float, float, const string&, const string&  >(),
            py::arg("opacity")=1, py::arg("shininess")=0.1, py::arg("emission_strength")=0, 
            py::arg("emission_color")="black", py::arg("name")="defaultMaterial")
        .def_property("name", &CMaterial::GetLabel, &CMaterial::SetLabel, "Name: string")
        .def_property("opacity", &CMaterial::GetOpacity, &CMaterial::SetOpacity,
            "Opacity (0.0-1.0)")
        .def_property("shininess", &CMaterial::GetShininess, &CMaterial::SetShininess,
            "Shininess (0.0-1.0)")
        .def_property("emission_strength", &CMaterial::GetEmissionStrength, &CMaterial::SetEmissionStrength,
            "Emission Strength (0.0-1.0)")
        .def_property("emission_color", &CMaterial::GetEmissionColor, &CMaterial::SetEmissionColor,
            "Emission Color. See https://htmlcolorcodes.com/color-names/ for list of all available colors.");

    py::class_<CModelComponent> (m, "BaseMesh", R"mydelimiter(
Inheritable parent class for : vivid3d.Mesh, vivid3d.PointCloud, vivid3d.Lines

Attributes
----------
name : str
    Optional name of the object, not required to be unique.
n_polygons : number
    # of vertices
n_vertices : number
    # of vertices
material : vivid3d.Material
    The material instance used by 3D renderers
colormap : matplotlib.colors.ColorMap
    The Colormap instance used to map scalar data to colors.
    See https://matplotlib.org/stable/gallery/color/colormap_reference for list of all supported colormaps.
opacity : number
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.

Methods
-------
set_color(color: string)
    Set the mesh texture to a single color. See https://htmlcolorcodes.com/color-names/ for list of all available colors.
transform(matrix: ndarray(3,3))
    Transform the Mesh by matrix
rotate(norm_vec: Point3D, radians_angle: float)
    Rotate the Mesh around the norm_vec by radians_angle
scale(scale_vec: Point3D)
    Scale the Mesh by scale_ved
move(direction_vec: Point3D)
    Move the Mesh by direction_vec
show(height=600) -> IPython.display.HTML or str
    Render and view the mesh in a viewer window.
export(output_file: str, file_type='glb2') -> BlobData or None
    Writes Component to output_file with given file format. See __reference__ for list of all supported file formats.
    If no output_file provided, this function will return a blob file in the given file format.

)mydelimiter")
    .def(py::init<const CModelComponent&> (),
        "Copy Constructor for BaseMesh",
        py::arg("model_component"))
    .def_property("label", &CModelComponent::GetLabel, &CModelComponent::SetLabel)
    .def_property_readonly("n_polygons", &CModelComponent::GetFacesCount, "# of polygons")
    .def_property_readonly("n_vertices", &CModelComponent::GetPointsCount, "# of vertices")
    .def_property("material", &CModelComponent::GetMaterial, &CModelComponent::SetMaterial, "The material instance used by 3D renderers")
    .def_property("colormap", &CModelComponent::GetColorMap, &CModelComponent::SetColorMap, "The Colormap instance used to map scalar data to colors.")
    .def_property("opacity", &CModelComponent::GetOpacity, &CModelComponent::SetOpacity,
        "Opacity (0.0-1.0)")
    .def("set_color", &CModelComponent::SetColor,
         R"mydelimiter(
Set the mesh texture to a single color. 

Parameters
----------
color : str
    See https://htmlcolorcodes.com/color-names/ for list of all available colors.

        )mydelimiter", py::arg("color"))
    .def("transform", py::overload_cast<const array<CPoint, 3>&>(&CModelComponent::TransformMesh),
         "Transform Mesh by transformation matrix", py::arg("matrix"))
    .def("rotate", &CModelComponent::RotateMesh,
R"mydelimiter(
Rotate the Mesh points around a normal vector by an angle in radians

Parameters
----------
norm_vec : Point3D (normalized)
    Unit vector from (0,0,0) of the BaseMesh
radians_angle : float
    The angle to rotate in radians

        )mydelimiter", py::arg("norm_vec"), py::arg("radians_angle"))
    .def("move", &CModelComponent::MoveMesh,
R"mydelimiter(
Moves Mesh in X,Y,Z by direction(X,Y,Z)

Parameters
----------
direction : Point3D
    The vector to move by

        )mydelimiter", py::arg("direction"))
    .def("scale", &CModelComponent::ScaleMesh,
        R"mydelimiter(
Scales Mesh in X,Y,Z by scale(X,Y,Z)

Parameters
----------
scale : Point3D
    The vector to scale by

        )mydelimiter", py::arg("scale"))
    .def("export", [](CModelComponent arSelf, string output_file, string file_type) {
            if (output_file.empty()) {
                return arSelf.ExportToBlob(file_type);
            } else {
                arSelf.Export(output_file, file_type);
            }
        }, R"mydelimiter(
Writes Mesh to output_file with given file format.

If no output_file provided, this function will return a blob file in the given file format.

Parameters
----------
output_file : str, default: ""
    File Directory to export to
file_type : str, default: 'glb2'
    File format to export to

Returns
-------
blob : vivid3d.BlobData or None
    Object containing exported model file blobs - if output_file is empty - else None

        )mydelimiter", py::arg("output_file")="", py::arg("file_type") = "glb2")
    .def("show", [](CModelComponent& arSelf, int aHeight) {
            auto viewer = py::module_::import("vivid3d.viewer");
            return viewer.attr("show")(arSelf, aHeight);
        }, R"mydelimiter(
Render and view the mesh in a viewer window.

Parameters
----------
height : int, default: 600
    height in pixels to open the viewer

Returns
-------
html : str or IPython.display.HTML
    The html with embedded model.

        )mydelimiter", py::arg("height")=600)
    .def("__str__", [](const CModelComponent& arSelf) {
        return "vivid3d.BaseMesh\nName: " + arSelf.GetLabel() + "\nVertices: " + to_string(arSelf.GetPointsCount()) + "\nFaces: " + to_string(arSelf.GetFacesCount());
    });

// Main Classes
    py::class_<CMesh, CModelComponent>(m, "Mesh", R"mydelimiter(
Triangular Mesh class that holds polygonal surfaces.

Methods
-------
smooth(n_iterations=5, opacity_weight=0.7, beta_weight=0.0)
    Smooths the surface by HC Laplacian Algorithm. 
    See http://informatikbuero.com/downloads/Improved_Laplacian_Smoothing_of_Noisy_Surface_Meshes.pdf for reference.
reduce(decimation_percent=0.5,max_error=0.1)
    Reduces Poly-Count of smoothed meshes. Prefer to create smaller models as it tends to have a cleaner result.
    See __example__ for down-sampling datasets.

        )mydelimiter")
    .def(py::init<const CMesh &> (),
        "copy constructor for Mesh", py::arg("mesh"))
    .def("reduce", &CMesh::Reduce, R"mydelimiter(
Reduces Poly-Count of smoothed meshes.

Reduces file size while trying to maintain the the shape as much as possible. It's recommended to not over do it.

Parameters
----------
decimation_percent : (0.0-1.0), default: 0.5
    Reduce the model polygon count to this percent.
max_error : (0.0-1.0), default = 0.5
    Maximum error in reduction. Recommended to not touch.

Notes
------
Prefer to create smaller models as it tends to have a cleaner result.
See __example__ for down-sampling datasets.

            )mydelimiter", py::arg("decimation_percent") = 0.5, py::arg("max_error") = 0.5)
        .def("smooth", &CMesh::LaplacianSmooth, R"mydelimiter(
Smooths the surface by HC Laplacian Algorithm. Recommended to use default values.

See http://informatikbuero.com/downloads/Improved_Laplacian_Smoothing_of_Noisy_Surface_Meshes.pdf for reference.

Parameters
----------
n_iterations : number, default: 5
    Number of iterations to smooth by
alpha_weight : (0.0-1.0), default: 0.7
    Percent to move the point by each iteration
beta_weight : (0.0-1.0), default: 0.0
    Percent to hold the point pack to original position. Used for high iteration smoothing, to retain total volume.
    If n_iterations > 20, recommended beta_weight = 0.5

        )mydelimiter", py::arg("n_iterations") = 5, py::arg("alpha_weight") = 0.7, py::arg("beta_weight") = 0.0);

    // TODO: Add color_field support to lines as in points.
    py::class_<CLines, CModelComponent>(m, "Lines", R"mydelimiter(
Lines class that holds 3d lines

Useful for plotting vector fields or trajectories.

Methods
-------
add_line(line: Point3DArray)
    Add a new line

        )mydelimiter")
    .def(py::init<normal_float, const string&>(), R"mydelimiter(
Lines Empty Constructor

Parameters
----------
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: "defaultMaterial"
    Optional name of the object, not required to be unique.

        )mydelimiter", py::arg("opacity")=1., py::arg("name")="")
//    .def(py::init<const vector<CPoint>&, const normal_float, const string& >(), R"mydelimiter(
//Lines Constructor
//
//Parameters
//----------------
//line : Point3DArray
//opacity : normalized float, default: 1.0
//Opacity of Component
//name : str, default: ""
//Label of Component
//        )mydelimiter", py::arg("line"), py::arg("opacity")=1., py::arg("name")="")
////            .def(py::init<const vector<vector<CLines>>&, const coord_t, const string&>(),
////                 "Constructor for Lines",
////                 py::arg("array_of_lines"), py::arg("opacity")=1., py::arg("name")="")
    .def(py::init<const CLines&>(),
         "Copy Constructor for Lines",
         py::arg("Lines"))
    .def("add_line", &CLines::AddLine,
         "Add another line",
         py::arg("line"));
//            .def("add_lines", &CLines::AddLine,
//                 "Add array of lines",
//                 py::arg("points_matrix"));

//TODO: Add VoronoiVolume analysis shit here.
py::class_<CVoronoiVolume>(m, "VoronoiVolume", R"mydelimiter(
VoronoiVolume class for Volumetric analysis and iso-surface extraction.

Methods
-------
iso_surface(mask: array_like(bool, n), label="isosurface", opacity=1)
    Extracts iso-surface by boolean mask of same size as input points, and returns vivid3d.Mesh instance.

        )mydelimiter")
    .def(py::init<const vector<CPoint>&, vector<normal_float>&, normal_float, normal_float, coord_t>(), R"mydelimiter(
Generates 3D Voronoi Volume from input points and scalar color_field.
See https://en.wikipedia.org/wiki/Voronoi_diagram for reference.

Parameters
----------
points : Point3DArray(n,3)
    array_like of 3D Points of shape (n,3)
color_field : array_like(n)[float], default: zeros(n)
    array_like of scalars to use for color_field of shape (n). If left empty will default to array of zeros.
color_field_min : float, default: 0.0
    Values below this will be clamped to this minimum. If null the minimum is automatically calculated
color_field_max : float, default: 0.0
    Values above this will be clamped to this maximum. If null the maximum is automatically calculated
noise_displacement : float, default: 0.0
    Sometimes helps fix edge cases where the algorithm fails. Use sparingly.

        )mydelimiter", py::arg("points"), py::arg("color_field") = vector<normal_float>(0), py::arg("color_field_min") = 0,
        py::arg("color_field_max") = 0, py::arg("noise_displacement") = 0)
    .def(py::init<const CVoronoiVolume &> (),
        "Copy constructor for VoronoiVolume",
        py::arg("voronoi_volume"))
    .def("iso_surface", &CVoronoiVolume::MaskMesh, R"mydelimiter(
Extracts iso-surface by boolean mask of same size as input points.

Only faces between a True and False point will be included in the final mesh. 
For a visual explanation, see https://vivid.readthedocs.io/en/latest/userguide/voronoi.html#the-voronoi-algorithm

Parameters
----------
mask : array_like(n)[bool]
    An (n, 1) shaped boolean array for creating the iso-surface.
    Only faces between a True and False point will be included in the final mesh.
name : str, default: "isosurface"
    Optional name of the object, not required to be unique.
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.

Returns
-------
vivid3d.Mesh
    The resulting iso-surface

        )mydelimiter", py::arg("mask"), py::arg("name") = "isosurface", py::arg("opacity") = 1);


    py::class_<CPointCloud, CModelComponent>(m, "PointCloud", R"mydelimiter(
PointCloud class for both surface and volumetric point clouds.

Can be analyzed with 3D Voronoi for iso-surface extraction.

Methods
-------
add_points(points: Point3DArray(n,3), color_field: array_like(n), color_field_min=0, color_field_max=0)
    Append the points to the existing point cloud.
add_noise(max_displacement: float)
    Moves particles randomly by uniform distribution[-max_displacement, max_displacement] along x,y,z
to_volume(noise_displacement=0)
    Generate 3D Volume using the Voronoi algorithm.
    See https://en.wikipedia.org/wiki/Voronoi_diagram for explanation.
        )mydelimiter")
    .def(py::init<normal_float, const std::string&>(),
        "Empty constructor for PointCloud",
        py::arg("opacity") = 1, py::arg("name")= "VIVID_POINT_CLOUD")
    .def(py::init<const std::vector<CPoint>&, const std::string&, normal_float, const std::string&>(),  R"mydelimiter(
PointCloud constructor from points and scalar color_field

Parameters
----------
points : Point3DArray(n,3)
    array_like of 3D Points of shape (n,3)
color : str, default: 'white'
    Color to apply to all the points.
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: 'pointCloud'
    Optional name of the object, not required to be unique.
        )mydelimiter", py::arg("points"), py::arg("color")="white", py::arg("opacity") = 1, py::arg("name")= "pointCloud")
    .def(py::init<const std::vector<CPoint>&, vector<normal_float>&, normal_float, normal_float, normal_float, const std::string&>(), R"mydelimiter(
PointCloud constructor from points and scalar color_field

Parameters
----------
points : Point3DArray(n,3)
    array_like of 3D Points of shape (n,3)
color_field : array_like(n)[float], default: zeros(n)
    array_like of scalars to use for color_field of shape (n). If left empty will default to array of zeros.
color_field_min : float, default: 0.0
    Values below this will be clamped to this minimum. If null the minimum is automatically calculated
color_field_max : float, default: 0.0
    Values above this will be clamped to this maximum. If null the maximum is automatically calculated
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: 'pointCloud'
    Optional name of the object, not required to be unique.
            )mydelimiter", py::arg("points"), py::arg("color_field") = vector<normal_float>(0),
        py::arg("color_field_min") = 0, py::arg("color_field_max") = 0, py::arg("opacity") = 1, py::arg("name")= "pointCloud")
    .def(py::init<const CPointCloud&>(),
        "Copy Constructor for Point Cloud",
        py::arg("point_cloud"))
    .def("add_points", &CPointCloud::AddPoints,
        "Add Points to the Point Cloud",
        py::arg("points"), py::arg("color_field") = vector<normal_float>(0), py::arg("color_field_min") = 0, py::arg("color_field_max") = 0)
    .def("add_noise", &CPointCloud::Noisify, R"mydelimiter(
Moves particles randomly by uniform distribution[-max_displacement, max_displacement] along x,y,z axes.

Useful for breaking up grid-like appearance of PointClouds generated with grid based data.

Parameters
----------
max_displacement : float
    Maximum amount to move in along axes. It is recommended to keep below the cell size.

        )mydelimiter", py::arg("max_displacement"))
    .def("to_voronoi", &CPointCloud::CreateVoronoiVolume, R"mydelimiter(
Generate 3D Volume using the Voronoi algorithm.

See https://en.wikipedia.org/wiki/Voronoi_diagram for explanation.

Parameters
----------
noise_displacement : float, default: 0.0
    Sometimes helps fix edge cases where the algorithm fails. Use sparingly.

Returns
-------
volume : vivid3d.VoronoiVolume
    The calculated 3D Cell volume

            )mydelimiter", py::arg("noise_displacement") = 0);

    py::class_<CModel>(m, "Model", R"mydelimiter(
Model class for combining multiple BaseMesh instances into a single scene.

Attributes
----------
meshes : list[vivid3d.BaseMesh]
    List of meshes held
n_meshes : number
    Number of meshes held
n_polygons : number
    Number of total polygons
n_vertices : number
    Number of total vertices

Methods
-------
add_mesh(mesh: vivid3d.BaseMesh)
    Add Mesh, PointCloud, or Lines object to model
add_meshes(mesh: vivid3d.BaseMesh)
    Add Meshes, PointCloud's, or Lines objects to model
export(output_file='', file_type='glb2')
    Default exporter supporting most common 3D file types. For full list of supported file formats see: __reference__
    Returns vivid3d.BlobData if output_file is empty.
export_to_obj(output_file: str, with_texture=True)
    Backup exporter if the main export() function fails.
show(height=600)
    Render and view the mesh in a viewer window.

    )mydelimiter")
    .def(py::init<> (), "Empty constructor for model")
    .def(py::init<vector<CModelComponent>& >(),
         "constructor for CModel, from meshes, lines, or point clouds",
         py::arg("meshes"))
    .def(py::init<CModelComponent& >(),
         "constructor for CModel, from mesh, line, or point cloud",
         py::arg("mesh"))
    .def("add_meshes", &CModel::AddMeshes, R"mydelimiter(
Add Meshes, PointClouds, or Lines objects to model

Parameters
----------
meshes : array_like[vivid3d.BaseMesh]
    List of vivid3d.Mesh, vivid3d.PointCloud, or vivid3d.Lines

        )mydelimiter", py::arg("meshes"))
    .def("add_mesh", &CModel::AddMesh, R"mydelimiter(
Add Mesh, PointCloud, or Lines object to model

Parameters
----------
mesh : vivid3d.BaseMesh
    Instance of vivid3d.Mesh, vivid3d.PointCloud, or vivid3d.Lines

        )mydelimiter", py::arg("mesh"))
    .def_property_readonly("meshes", &CModel::GetMeshes,
         "Returns the BaseMeshes held")
    .def_property_readonly("n_meshes", &CModel::GetNumMeshes,
        "Number of BaseMeshes held")
    .def_property_readonly("n_polygons", &CModel::GetNumMeshes,
        "Total number of polygons in all Meshes")
    .def_property_readonly("n_vertices", &CModel::GetNumMeshes,
        "Total number of vertices in all Meshes")
    .def("export_to_obj", &CModel::ExportToObj, R"mydelimiter(
Writes Model to output_file with in standard OBJ format.
Backup exporter if the main export() function fails.

Parameters
----------
output_file : str, default: ""

File Directory to export to


        )mydelimiter", py::arg("output_file"), py::arg("with_texture") = 1)
    .def("export", [](CModel &arSelf, string output_file, string file_type) {
        if (output_file.empty()) {
            return arSelf.ExportToBlob(file_type);
        } else {
            arSelf.Export(output_file, file_type);
        }
    }, R"mydelimiter(
Writes Model to output_file with given file_type format.

For full list of supported file formats, see: __reference__
If no output_file provided, this function will return a blob file in the given file format.

Parameters
----------
output_file : str, default: ""
    File Directory to export to
file_type : str, default: 'glb2'
    File format to export to

Returns
-------
blob : vivid3d.BlobData
    Object containing exported model file blobs - only returned if output_file is empty

        )mydelimiter", py::arg("output_file")="", py::arg("file_type") = "glb2")
    .def("show", [](CModel &arSelf, int aHeight) {
            auto viewer = py::module_::import("vivid3d.viewer");
            return viewer.attr("show")(arSelf, aHeight);
        }, R"mydelimiter(
Render and view the model in a viewer window.

Parameters
----------
height : int, default: 600
    height in pixels to open the viewer

Returns
-------
html : str or IPython.display.HTML
    The HTML with embedded model

        )mydelimiter", py::arg("height")=600)
    .def("__str__", [](const CModel &arSelf){ return "vivid3d.Model\nn_meshes: " + to_string(arSelf.GetNumMeshes()) + "\nn_polygons: " + to_string(arSelf.GetNumPolygons()) + "\nn_vertices: " + to_string(arSelf.GetNumVertices()); });

    py::class_<CAnimation> Animation(m,"Animation", R"mydelimiter(
Class for animations

        )mydelimiter");
    Animation.def(py::init<> (), "default constructor for Animation")
    .def(py::init<const CModel &> (),
         "constructor for Animation from a single Model",
         py::arg("model"))
    .def(py::init<const vector<CModel> &> (),
         "constructor for Animation from an np array of Models",
         py::arg("models"))
    .def(py::init<const CAnimation &> (),
         "copy constructor for animation",
         py::arg("animation"))
    .def_property("duration", &CAnimation::GetDuration, &CAnimation::SetDuration, "duration(in ticks)")
    .def_property("ticks_per_second", &CAnimation::GetTicksPerSecond, &CAnimation::SetTicksPerSecond, "ticks")
    .def("get_models", &CAnimation::GetModels,
         "getter function for models in the animation")
    .def("add_models", static_cast<void (CAnimation::*)(const CModel &)>(&CAnimation::AddModels),"add a model to animation", py::arg("models"))
    .def("add_models", static_cast<void (CAnimation::*)(const vector<CModel> &)>(&CAnimation::AddModels),"add models to animation", py::arg("models"))
    .def("set_move_animation", &CAnimation::SetMoveAnim,
         "set movement animation for index model",
         py::arg("index"), py::arg("move_vector"))
    .def("set_rotate_animation", &CAnimation::SetRotateAnim,
         "set rotate animation for index model",
         py::arg("index"), py::arg("anguler_rotate"))
    .def("set_scale_animation", &CAnimation::SetScaleAnim,
         "set scale animation for index model",
         py::arg("index"), py::arg("scale_vector"))
    .def("get_move_animation", &CAnimation::GetMoveAnim,
         "get movement animation for given model index",
         py::arg("index"))
    .def("get_rotate_animation", &CAnimation::GetRotateAnim,
         "get rotate animation for given model index",
         py::arg("index"))
    .def("get_scale_animation", &CAnimation::GetScaleAnim,
         "get scale animation for given model index",
         py::arg("index"))
    .def("export", &CAnimation::Export,
         "Exports animation to selected file format",
         py::arg("output_file"), py::arg("file_format") = "gltf2");


    py::class_<CStopMotionAnimation>(m,"StopMotionAnimation", Animation, R"mydelimiter(
Class for stop motion animations

    )mydelimiter")
        .def(py::init<> (), "default constructor for StopMotionAnimation")
        .def(py::init<const CModel &, double> (),
            "constructor for StopMotionAnimation from a single Model",
            py::arg("model"), py::arg("seconds_per_frame") = 2) //py::arg("model"), py::arg("mSecondsPerFrame")
        .def(py::init<const vector<CModel> &, double> (),
            "constructor for StopMotionAnimation from an np array of Models",
            py::arg("models"), py::arg("seconds_per_frame") = 2) //py::arg("models"), py::arg("mSecondsPerFrame")
        .def(py::init<const CAnimation &, double> (),
            "constructor for StopMotionAnimation from animation",
            py::arg("animation"), py::arg("seconds_per_frame") = 2) // py::arg("animation"), py::arg("mSecondsPerFrame")
        .def(py::init<const CStopMotionAnimation &> (),
            "copy constructor for StopMotionAnimation",
            py::arg("animation"))
        .def_property("seconds_per_frame", &CStopMotionAnimation::GetSecondsPerFrame, &CStopMotionAnimation::SetSecondsPerFrame,
             "Seconds per frame");


    //one liners
    m.def("make_model", py::overload_cast<const std::vector<CPoint> &, const std::vector<bool> &, const std::string &,
        std::vector<normal_float> &, normal_float , normal_float , const string& ,
        normal_float, const std::string&, coord_t>(&vivifyMesh), R"mydelimiter(
one function to rule them all

makes a model from one line, if an output path is given the functions writes the model to file.

Parameters
----------
input_points : array[array[float]]
    The input point data in x,y,z form.
mask : array[bool]
    Boolean mask of true and false points
output_path : str, default: ""
    Path and name for output file
    If left as None no Module will be written to file.
color_field : array[float], default: []
    Vector containing the color field of each point
    If left empty color will be basic white
color_field_min : float, optional
    Set minimum value for color_field, anything below will be set to color_field_min
    If left empty will be set to min(color_field)
color_field_max : float, optional
    Set max value for color_field, anything below will be set to color_field_max
    If left empty will be set to max(color_field)
name : str, default: "VIVID_MODEL"
    Optional name of the object, not required to be unique.
opacity : float, default: 1
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
file_type : str, default: "gltf2"
    File format for export, out of Assimp supported formats
noise_displacement : float, default: 0.001
    The Voronoi algorithm struggles with equidistant point data, a small noise displacement improves algorithm speed

Returns
-------
vivid3d.Model
    Output model.

See Also
----------------
vivid3d.Model
    Main class holding 3d models data in the vivid3d package, also see examples.

        )mydelimiter",
        py::arg("input_points"),
        py::arg("mask"),
        py::arg("output_path") = "",
        py::arg("color_field") = vector<normal_float>(0),
        py::arg("color_field_min") = 0,
        py::arg("color_field_max") = 0,
        py::arg("name")= "VIVID_MODEL",
        py::arg("opacity") = 1,
        py::arg("file_type") = "gltf2",
        py::arg("noise_displacement") = 0.001);


//    m.def("make_model", py::overload_cast<const std::vector<CPoint> &, const std::vector<normal_float> &, normal_float,
//        const std::string &, std::vector<normal_float> &, normal_float , normal_float , const string& ,
//        normal_float , const std::string&, coord_t>(&vivifyMesh), R"mydelimiter(
//        one function to rule them all
//
//        makes a model from one line, if an output path is given the functions writes the model to file.
//        )mydelimiter",
//        py::arg("input_points"),
//        py::arg("surface_field"),
//        py::arg("surface_threshold"),
//        py::arg("output_path") = "",
//        py::arg("color_field") = vector<normal_float>(0),
//        py::arg("color_field_min") = 0,
//        py::arg("color_field_max") = 0,
//        py::arg("name")= "VIVID_MODEL",
//        py::arg("opacity") = 1,
//        py::arg("file_type") = "gltf2",
//        py::arg("noise_displacement") = 0.001);
//
//    m.def("make_model", py::overload_cast<const std::vector<CPoint> &, const std::vector<std::vector<bool>> &, const std::string &,
//        std::vector<normal_float> &, normal_float , normal_float , const std::string& ,
//        vector<normal_float> &, const std::string&, coord_t>(&vivifyModel), R"mydelimiter(
//        one function to rule them all
//
//        makes a model from one line, if an output path is given the functions writes the model to file.
//        )mydelimiter",
//        py::arg("input_points"),
//        py::arg("masks"),
//        py::arg("output_path") = "",
//        py::arg("color_field") = vector<normal_float>(0),
//        py::arg("color_field_min") = 0,
//        py::arg("color_field_max") = 0,
//        py::arg("name")= "VIVID_MODEL",
//        py::arg("opacity") = vector<normal_float>(0),
//        py::arg("file_type") = "gltf2",
//        py::arg("noise_displacement") = 0.001);
//
//
//    m.def("make_model", py::overload_cast<const std::vector<CPoint> &, const std::vector<normal_float> &, std::vector<normal_float> &,
//        const std::string &, std::vector<normal_float> &, normal_float , normal_float , const std::string&,
//        std::vector<normal_float> &, const std::string&, coord_t >(&vivifyModel), R"mydelimiter(
//        one function to rule them all
//
//        makes a model from one line, if an output path is given the functions writes the model to file.
//        )mydelimiter",
//        py::arg("input_points"),
//        py::arg("surface_field"),
//        py::arg("surface_thresholds"),
//        py::arg("output_path") = "",
//        py::arg("color_field") = vector<normal_float>(0),
//        py::arg("color_field_min") = 0,
//        py::arg("color_field_max") = 0,
//        py::arg("name")= "VIVID_MODEL",
//        py::arg("opacity") = vector<normal_float>(0),
//        py::arg("file_type") = "gltf2",
//        py::arg("noise_displacement") = 0.001);


    //Shapes:
    m.def("create_plane", &CreatePlaneMesh,R"mydelimiter(
Create a 2D Plane mesh

Parameters
----------
position : Point3D, default: [0,0,0]
    The center of the mesh.
size : float, default: 1.0
    The distance of the vertices from the position.
color : str, default: 'lightgray'
    The color of the resulting mesh.
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: 'plane'
    Optional name of the object, not required to be unique. 

Returns
-------
mesh : vivid3d.Mesh
    The 2D Plane mesh

        )mydelimiter", py::arg("position") = make_cpoint(0), py::arg("size") = 1,
        py::arg("color")=DEFAULT_COLOR_SHAPES, py::arg("opacity") = 1., py::arg("name")="plane");
    m.def("create_cube", &CreateCubeMesh, R"mydelimiter(
Create a Cube mesh

Parameters
----------
position : Point3D, default: [0,0,0]
    The center of the mesh.
size : float, default: 1.0
    The distance of the vertices from the position.
color : str, default: 'lightgray'
    The color of the resulting mesh.
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: 'cube'
    Optional name of the object, not required to be unique.

Returns
-------
mesh : vivid3d.Mesh
    The cube mesh

        )mydelimiter", py::arg("position") = make_cpoint(0), py::arg("size") = 1, py::arg("color")=DEFAULT_COLOR_SHAPES, 
        py::arg("opacity") = 1., py::arg("name")="cube");
    m.def("create_box", &CreateBoxMesh, R"mydelimiter(
Create a Box mesh

Parameters
----------
position : Point3D, default: [0,0,0]
    The center of the mesh.
size : Point3D, default: [1,1,1]
    The distance of the vertices[X,Y,Z] from the position[X,Y,Z].
color : str, default: 'lightgray'
    The color of the resulting mesh.
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: 'box'
    Optional name of the object, not required to be unique.

Returns
-------
mesh : vivid3d.Mesh
    The box mesh

        )mydelimiter", py::arg("position") = make_cpoint(0), py::arg("size") = make_cpoint(1),
          py::arg("color")=DEFAULT_COLOR_SHAPES, py::arg("opacity") = 1., py::arg("name")="box");
    m.def("create_sphere", &CreateSphereMesh, R"mydelimiter(
Create a Sphere mesh

Parameters
----------
position : Point3D, default: [0,0,0]
    The center of the mesh.
radius : float, default: 1.0
    The radius from the position.
num_of_meridians : int, default: 20
    How many horizontal slices to subdivide the sphere mesh.
num_of_parallels : int, default: 20
    How many vertical slices to subdivide the sphere mesh.
color : str, default: 'lightgray'
    The color of the resulting mesh.
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: 'sphere'
    Optional name of the object, not required to be unique.

Returns
-------
mesh : vivid3d.Mesh
    The sphere mesh

        )mydelimiter", py::arg("position")=make_cpoint(0),py::arg("radius")=1,py::arg("num_of_meridians")=20,py::arg("num_of_parallels")=20,
        py::arg("color")=DEFAULT_COLOR_SHAPES, py::arg("opacity")=1., py::arg("name")="sphere");
    m.def("create_ellipsoid", &CreateEllipsoidMesh, R"mydelimiter(
Create an Ellipsoid mesh

Parameters
----------------
position : Point3D, default: [0,0,0]
    The center of the mesh.
radii : Point3D
    The major, middle, and minor axis radii.
num_of_meridians : int, default: 20
    How many horizontal slices to subdivide the sphere mesh.
num_of_parallels : int, default: 20
    How many vertical slices to subdivide the sphere mesh.
major_axis : Point3D
    The unit vector for ellipsoid major axis.
middle_axis : Point3D
    The unit vector for ellipsoid middle axis.
minor_axis : Point3D
    The unit vector for ellipsoid minor axis.
color : str, default: 'lightgray'
    The color of the resulting mesh.
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: 'ellipsoid'
    Optional name of the object, not required to be unique.

Returns
----------------
mesh : vivid3d.Mesh
    The ellipsoid mesh

        )mydelimiter", py::arg("position")=make_cpoint(0),py::arg("radii"),py::arg("num_of_meridians")=20,py::arg("num_of_parallels")=20,
        py::arg("major_axis"), py::arg("middle_axis"), py::arg("minor_axis"),
        py::arg("color")=DEFAULT_COLOR_SHAPES, py::arg("opacity")=1., py::arg("name")="ellipsoid");
    m.def("create_arrow", &CreateArrowMesh, R"mydelimiter(
Create an Arrow mesh

Parameters
----------------
position : Point3D, default: [0,0,0]
    The center of the mesh.
direction : Point3D
    The direction and length of the arrow from the position.
width : float, default: 0.5
    The width of the arrow mesh. 
pointer_chest_ratio : float, default: 0.4
    The pointer to chest ratio.
color : str, default: 'lightgray'
    The color of the resulting mesh.
opacity : (0.0-1.0), default: 1.0
    Float in the range of 0.0 - 1.0 indicating how transparent the material is. A value of 0.0 indicates fully transparent, 1.0 is fully opaque.
name : str, default: 'arrow'
    Optional name of the object, not required to be unique.

Returns
----------------
mesh : vivid3d.Mesh
    The arrow mesh

        )mydelimiter", py::arg("position")=make_cpoint(0), py::arg("direction"),py::arg("width")=0.5, py::arg("pointer_chest_ratio")=0.4,
        py::arg("color")=DEFAULT_COLOR_SHAPES, py::arg("opacity") = 1, py::arg("name")="arrow");
    m.def("create_grid", &CreateGrid,R"mydelimiter(
Create a Grid mesh, origin is at [0,0,0]

Parameters
----------------
size : float, default: 1.0
    The size of the grid from [0,0,0]
num_of_ticks : int, default: 5
    The number of ticks between each major tick.
tick_size: float, default: 1
    The size of ticks.

Returns
----------------
vivid3d.Lines
    The grid mesh

        )mydelimiter", py::arg("size")=10, py::arg("num_of_ticks")=5, py::arg("tick_size")=1);

    //SurfByFunc:

    // Utils:
    py::enum_<ELogCode>(m, "LogCode")
        .value("LOG_ERROR", LOG_ERROR)
        .value("LOG_WARNING", LOG_WARNING)
        .value("LOG_VIVID", LOG_VIVID)
        .value("LOG_INFO", LOG_INFO)
        .value("LOG_DEBUG", LOG_DEBUG)
        .export_values();

    m.def("config_logger", &ConfigLogging, R"mydelimiter(
Configure the VIVID3D in-built Logging Process

Parameters
----------
write_to_console : bool, default: True
    Print all Logging output to console.
log_level : vivid3d.LogCode, default: LOG_VIVID
    Log all messages above or at this log level.
write_to_file : bool, default: false
    Write all Logging output to file.
log_file_name : str, default: 'VIVID_LOG.txt'
    File name for Logging.

        )mydelimiter", py::arg("write_to_console") = true, py::arg("log_level") = LOG_VIVID, 
            py::arg("write_to_file") = false, py::arg("log_file_name") = "VIVID_LOG.txt");
}
