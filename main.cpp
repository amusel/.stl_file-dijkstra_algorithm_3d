#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>

using namespace std;

struct 	point		//structure of point(x,y,z) and flag
{					//which is used to mark the use of point
	float	x;
	float	y;
	float	z;
	int		flag;

	bool operator == (const point& p1)
	{
		if (x == p1.x && y == p1.y && z == p1.z)
			return true;
		return false;
	}

	const bool operator < (const point &v1)		//overloading for sort()
	{
		double one(sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2)));
		double two(sqrt(pow(v1.x, 2) + pow(v1.y, 2) + pow(v1.z, 2)));

		return (one < two);
	}

	float& operator[](unsigned int index)
	{
		return ((float*)this)[index];
	}

	point(float xx, float yy, float zz, int f = 0)
	{
		x = xx;
		y = yy;
		z = zz;
		flag = f;
	}

	point() : x(), y(), z(), flag(0)
	{}
};

float	parse_float(ifstream& s)	//reading of one float
{
	char f_buf[sizeof(float)];
	s.read(f_buf, 4);
	auto fptr = (float*)f_buf;
	return (*fptr);
}

point	parse_point(ifstream& s)	//reading of point
{
	point ret;
	ret.x = parse_float(s);
	ret.y = parse_float(s);
	ret.z = parse_float(s);
	return (ret);
}

vector<point> 	parse(string stl_file_name)
{
	ifstream stl_file(stl_file_name.c_str(), ios::in | ios::binary);
	if (!stl_file.is_open())
	{
		cout << "ERROR: COULD NOT READ FILE" << endl;
		exit(EXIT_FAILURE);
	}

	char header[80] = "";
	char n_triangles[4];
	stl_file.read(header, 80);				//skiping header of .stl
	stl_file.read(n_triangles, 4);			//reading the amount of triangles
	auto r = (unsigned int *)n_triangles;
	unsigned int num_triangles = *r;
	vector<point> all_points(num_triangles * 3);	//every triangle has 3 points
	int j = 0;
	for (unsigned int i = 0; i < num_triangles; i++)//reading of every point
	{
		parse_point(stl_file);				//skipping the normal of vector
		all_points[j++] = parse_point(stl_file);
		all_points[j++] = parse_point(stl_file);
		all_points[j++] = parse_point(stl_file);
		char skip[2];
		stl_file.read(skip, 2);				//skipping additional info
	}

	return (all_points);
}

vector<vector<point>> 	into_vectors(vector<point> tri, point s, point e)
{
	vector<vector<point>> vectors(tri.size(), vector<point>(1));
	unsigned int c = 0;
	int start = -1, end = -1;			//flags for source and target points

	for (unsigned int i = 0; i<tri.size(); i++)	//going through every point
	{
		if (tri[i][3] == 0)				//checking if point is unique
		{
			vectors[c][0] = tri[i];
			for (unsigned int j = i; j < tri.size(); j++) //recording all vectors of this point
				if (tri[j] == tri[i])
				{
					if (j % 3 == 0)			//every point in triangle has at least
					{						//two points to connect with
						vectors[c].push_back(tri[j + 1]);
						vectors[c].push_back(tri[j + 2]);
					}
					else if (j % 3 == 1)
					{
						vectors[c].push_back(tri[j - 1]);
						vectors[c].push_back(tri[j + 1]);
					}
					else
					{
						vectors[c].push_back(tri[j - 1]);
						vectors[c].push_back(tri[j - 2]);
					}
					if (j != i)
						tri[j][3] = 1;		//changing flag to not check this point again
				}

			sort(vectors[c].begin() + 1, vectors[c].end());		//deleting duplicates
			vectors[c].erase(unique(vectors[c].begin(), vectors[c].end()), vectors[c].end());

			if (fabs(tri[i][0] - s[0]) < 0.0001 && fabs(tri[i][1] - s[1]) < 0.0001 \
				&& fabs(tri[i][2] - s[2]) < 0.0001)			//can not find another way to compare float values
				start = c;
			if (fabs(tri[i][0] - e[0]) < 0.0001 && fabs(tri[i][1] - e[1]) < 0.0001 \
				&& fabs(tri[i][2] - e[2]) < 0.0001)
				end = c;
			vectors[c++].shrink_to_fit();
		}
	}

	vectors.resize(c);
	swap(vectors[0], vectors[start]);	//place source point in the beginning
	swap(vectors[vectors.size() - 1], vectors[end]);	//and target point in the end

	if (start != -1 && end != -1)	//checking whether source and target point are present
		return (vectors);			//in our array
	cout << "There is no connection between source point and target point!\n";
	exit(EXIT_FAILURE);
}

vector<point>			path_value(vector<vector<point>> vectors)
{
	vector<vector<float>>	value(vectors.size(), vector<float>(3));
	//matrix of path values
	int flag = 0;
	for (unsigned int i = 0; i<value.size(); i++)
	{
		value[i][0] = 1000000000;	//value of non-checked path
		value[i][1] = 0;			//0 = unchecked point
		value[i][2] = -1;			//previous point
	}
	value[0][0] = 0;				//value of start
	do
		for (unsigned int i = 0; i < vectors.size() - 1; i++)
			if (value[i][1] == 0 && value[i][0] < 1000000000)
			{
				flag = value[i][1] = 1;			//1 = checked point
				for (unsigned int j = 1; j < vectors[i].size(); j++)
					for (unsigned int k = 0; k < value.size(); k++)
						if (vectors[k][0] == vectors[i][j])
						{						//change value, if its less
							if (sqrtf(powf(vectors[i][0][0] - vectors[i][j][0], 2)\
								+ powf(vectors[i][0][1] - vectors[i][j][1], 2)\
								+ powf(vectors[i][0][2] - vectors[i][j][2], 2)) + value[i][0] < value[k][0])
							{
								value[k][0] = sqrtf(powf(vectors[i][0][0] - vectors[i][j][0], 2)\
									+ powf(vectors[i][0][1] - vectors[i][j][1], 2)\
									+ powf(vectors[i][0][2] - vectors[i][j][2], 2)) + value[i][0];
								value[k][2] = i;
							}
							break;
						}
			}
	while (flag--);

	if (value[value.size() - 1][2] == -1)
	{
		cout << "There is no connection between source point and target point!\n";
		exit(EXIT_FAILURE);
	}

	float distance = value[value.size() - 1][0];
	cout << "Shortest path value: " << distance << endl;

	vector<point>	path;
	float i = value.size() - 1;
	while (i != -1)			//recording all paths
	{
		path.push_back(vectors[i][0]);
		i = value[i][2];
	}
	path.shrink_to_fit();

	return (path);
}

int					main()
{
	string stl_file_name;
	point start, end;
	
	stl_file_name = "../sources/1.stl";
	start = {-17, -15, 0};
	end = {-6.999, -25, 10.001};
	if (start == end)
	{
		cout << "Target point == source point!";
		return (1);
	}
	
	auto all_points = parse(stl_file_name);		//reading of file, triangles points record
	auto vectors = into_vectors(all_points, start, end);	//finding and recording of all vectors
	auto path = path_value(vectors);			//finding Dijkstra the shortest path

	cout << "Path in points\n";
	for (int i = 0; i<path.size(); i++)		//paths output
		cout << path[i].x << " " << path[i].y << " " << path[i].z << endl;
	return (0);
}
