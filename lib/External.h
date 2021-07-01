#ifndef VIVID_VORONOI_H
#define VIVID_VORONOI_H

#include "Point.h"
#include "IndexedFace.h"
#include "mdMeshDecimator.h"

using namespace MeshDecimation; //TODO
namespace vivid
{

// TODO should have gotten Mesh object... by ref, also return value should be changed
std::pair<std::vector<CPoint>, std::vector<CIndexedFace> > DecimateMesh(std::vector<CPoint>& aPoints, std::vector<CIndexedFace> aFaces, int aTargetVerticesN, int aTargetTrianglesN, float aMaxError);

}; // namespace vivid
#endif //VIVID_VORONOI_H