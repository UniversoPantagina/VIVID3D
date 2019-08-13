
#include "External.h"

vector<Vector3D> ConvertToVorn(vector<CPoint> inputPoints) {
	vector<Vector3D> new_vec;
	for (vector<CPoint>::iterator it = inputPoints.begin(); it != inputPoints.end(); it++) {
		new_vec.push_back(Vector3D(it->GetX(), it->GetY(), it->GetZ()));
	}
	return new_vec;
}

pair<vector<Vector3D>, vector<vector<size_t>>> compute_vornoi(vector<CPoint> aInputPoints, double aBoxR) {
	vector<Vector3D> vorn_points = ConvertToVorn(aInputPoints);

	/////////////////////////////////////debug////////////////////////////////////////////////////
	//cout << vornPoints.size() << endl;
	//ofstream o;
	//o.open("../../test_models/voroni_input.txt");
	//o << "box size input:\n";
	//o << to_string(-box_R) + " " + to_string(-box_R) + " " + to_string(-box_R) + "\n";
	//o << to_string(box_R) + " " + to_string(box_R) + " " + to_string(box_R) + "\n";
	//o << "points input:\n";
	//for (vector<Vector3D>::iterator it = vornPoints.begin(); it != vornPoints.end(); it++) {
	//	o << to_string(it->x) + " " + to_string(it->y) + " " + to_string(it->z) + "\n";
	//}
	//o.close();
	/////////////////////////////////////////debug over/////////////////////////////////////////////

	Voronoi3D temp(Vector3D(-aBoxR, -aBoxR, -aBoxR), Vector3D(aBoxR, aBoxR, aBoxR));
	temp.Build(vorn_points);
	vorn_points = temp.GetFacePoints();

	vector<vector<size_t>> faces;
	size_t total_cells = temp.GetAllCellFaces().size();
	face_vec cell;
	point_vec face_points;
	size_t c_point1, c_point2;
	for (size_t i = 0; i < total_cells; i++) {
		cell = temp.GetCellFaces(i);
		for (face_vec::iterator face = cell.begin(); face != cell.end(); face++) {
			c_point1 = get<0>(temp.GetFaceNeighbors(*face));
			c_point2 = get<1>(temp.GetFaceNeighbors(*face));
			if (!(c_point1 < i) && !(c_point2 < i)) { //the face doent belong to a cell we read already
				faces.push_back(vector<size_t>());
				face_points = temp.GetPointsInFace(*face);
				for (point_vec::iterator point = face_points.begin(); point != face_points.end(); point++) {
					faces.back().push_back(*point);
				}
				faces.back().push_back(c_point1);
				faces.back().push_back(c_point2);
			}
		}
	}
	pair<vector<Vector3D>, vector<vector<size_t>>> output(vorn_points, faces);
	return output;
}

void CallBack(const char * msg)
{
	cout << msg;
}

pair<vector<CPoint>, vector<CIndexedFace>> DecimateMesh(vector<CPoint> aPoints, vector<CIndexedFace> aFaces, int aTargetVerticesN, int aTargetTrianglesN, float aMaxError) {
	//write the data to vec3 format
	vector< Vec3<float>> vertices;
	vector< Vec3<int>> triangles;
	Vec3<int> temp_triangle;
	for (vector<CIndexedFace>::iterator fIt = aFaces.begin(); fIt != aFaces.end(); fIt++) {
		temp_triangle = Vec3<int>((*fIt)[0], (*fIt)[1], (*fIt)[2], fIt->GetColor());
		triangles.push_back(temp_triangle); //, fIt->GetColor()
	}
	for (vector<CPoint>::iterator pIt = aPoints.begin(); pIt != aPoints.end(); pIt++) {
		vertices.push_back(Vec3<float>(pIt->GetX(), pIt->GetY(), pIt->GetZ(), 0));
	}
	//decimate model
	MeshDecimator myMDecimator;
	myMDecimator.SetCallBack(&CallBack);
	myMDecimator.Initialize(vertices.size(), triangles.size(), &vertices[0], &triangles[0]);
	myMDecimator.Decimate(aTargetVerticesN, aTargetTrianglesN, aMaxError);

	//get decimated data
	vector< Vec3<Float> > decimated_points;
	vector< Vec3<int> > decimated_triangles;
	decimated_points.clear();
	decimated_triangles.clear();

	myMDecimator.GetMeshData(&decimated_points, &decimated_triangles);

	//decimated_points.resize(myMDecimator.GetNVertices());
	//decimated_triangles.resize(myMDecimator.GetNTriangles());

	//write the data to Mesh format
	vector<CPoint> out_points;
	out_points.clear();
	vector<CIndexedFace> out_faces;
	//out_faces.clear();
	for (vector<Vec3<float>>::iterator pIt = decimated_points.begin(); pIt != decimated_points.end(); pIt++) {
		out_points.push_back(CPoint(pIt->X(), pIt->Y(), pIt->Z()));
	}
	for (vector<Vec3<int>>::iterator fIt = decimated_triangles.begin(); fIt != decimated_triangles.end(); fIt++) {
		out_faces.push_back(CIndexedFace((*fIt)[0], (*fIt)[1], (*fIt)[2], fIt->mQuan)); //, fIt->quan
	}
	pair<vector<CPoint>, vector<CIndexedFace>> output(out_points, out_faces);
	return output;
}