#include "object.h"

map<string, Object> objmap;
set<string> objname;
string cd;
map<string, Material> matname;

void loadbasket()
{
	glPushMatrix();
	glRotatef(-90, 0, 1, 0);
	glRotatef(-90, 0, 0, 1);
	glTranslatef(-75, 30.5, -4);
	for (set<string>::iterator it = objname.begin(); it != objname.end(); ++it)
	{
		Object temp = objmap[*it];
		setMaterial(matname[temp.material]);
		glBindTexture(GL_TEXTURE_2D, matname[temp.material].map);
		if (temp.row == 3) glBegin(GL_TRIANGLES);
		else glBegin(GL_QUADS);
		vector<int>::iterator iter = temp.faces.begin();
		if (temp.col == 1)
		{
			while (iter != temp.faces.end())
			{
				glVertex3f(temp.vertexs[*iter - 1].x/100, temp.vertexs[*iter - 1].y/100, temp.vertexs[*iter - 1].z/100);
				++iter;
			}
		}
		else if (temp.col == 2)
		{
			while (iter != temp.faces.end())
			{
				glTexCoord2f(temp.texcoords[*(iter + 1) - 1].first, temp.texcoords[*(iter + 1) - 1].second);
				glVertex3f(temp.vertexs[*iter - 1].x/100, temp.vertexs[*iter - 1].y/100, temp.vertexs[*iter - 1].z/100);
				iter += 2;
			}
		}
		else if (temp.col == 3 && !temp.texcoords.empty())
		{
			while (iter != temp.faces.end())
			{
				glNormal3f(temp.normals[*(iter + 2) - 1].x, temp.normals[*(iter + 2) - 1].y, temp.normals[*(iter + 2) - 1].z);
				//				glTexCoord2f(temp.texcoords.at(*(iter+1)-1).first,temp.texcoords.at(*(iter+1)-1).second);
				cout << *(iter + 1) - 1 << endl;
				glVertex3f(temp.vertexs[*iter - 1].x / 100, temp.vertexs[*iter - 1].y / 100, temp.vertexs[*iter - 1].z / 100);
				cout << "x is " << temp.vertexs[*iter - 1].x / 100 << " and y is " << temp.vertexs[*iter - 1].y / 100 << " and z is" << temp.vertexs[*iter - 1].z / 100;
				iter += 3;
			}
		}
		else
		{
			while (iter != temp.faces.end())
			{
				glNormal3f(temp.normals[*(iter + 2) - 1].x, temp.normals[*(iter + 2) - 1].y, temp.normals[*(iter + 2) - 1].z);
				glVertex3f(temp.vertexs[*iter - 1].x/100, temp.vertexs[*iter - 1].y/100, temp.vertexs[*iter - 1].z/100);
				iter += 3;
			}
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnd();
	}
	glPopMatrix();
}

void setMaterial(Material &mat)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat.ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat.diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat.specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat.emission);
	glMaterialf(GL_FRONT, GL_SHININESS, 100);
}

GLuint load_texture_LLC(const char *file_name)
{
	GLint width, height, total_bytes;
	GLuint last_texture_ID, texture_ID = 0;
	GLubyte *pixels = NULL;
	FILE *pFile;

	if ((pFile = fopen(file_name, "rb")) == NULL)
	{
		cout << "Read texture error" << endl;
		return 0;
	}

	fseek(pFile, 18, SEEK_SET);
	fread(&width, sizeof(width), 1, pFile);
	fread(&height, sizeof(height), 1, pFile);
	fseek(pFile, 54, SEEK_SET);

	total_bytes = (width * 3 + (4 - width * 3 % 4) % 4)*height;

	if ((pixels = (GLubyte *)malloc(total_bytes)) == NULL)
	{
		fclose(pFile);
		return 0;
	}

	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width) || !power_of_two(height) || width>max || height>max)
		{
			const GLint new_width = 1024;
			const GLint new_height = 1024;
			GLint new_total_bytes;
			GLubyte *new_pixels = NULL;

			new_total_bytes = (new_width * 3 + (4 - new_width * 3 % 4) % 4)*new_height;
			new_pixels = (GLubyte *)malloc(new_total_bytes);
			if (new_pixels == NULL)
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}

			gluScaleImage(GL_RGB, width, height, GL_UNSIGNED_BYTE, pixels, new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	glGenTextures(1, &texture_ID);
	if (texture_ID == 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *)&last_texture_ID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture_ID);

	free(pixels);
	fclose(pFile);
	return texture_ID;
}


/*
GLuint load_texture(char *fn)
{
GLint width, height, total_bytes;
GLubyte *pixels = NULL;
GLuint texture_ID;
GLint last_texture_ID = 0;

FILE *fp;
fp = fopen(fn,"rb");
if (fp == NULL){
cerr << "open file failed" << endl;
return 0;
}
fseek(fp, 0x0012, SEEK_SET);
fread(&width, 4, 1, fp);
fread(&height, 4, 1, fp);
if (width < 0)width = -width;
if (height < 0)height = -height;
fseek(fp, BMP_Header_Length, SEEK_SET);
{
GLint line_bytes = width * 3; //RGB 24bit
while (line_bytes % 4)line_bytes++;
total_bytes = line_bytes*height;
}
pixels = (GLubyte *)malloc(total_bytes);
if (pixels == NULL){
cerr << "malloc wrong pixels" << endl;
fclose(fp);
return 0;
}
int count;
count = fread(pixels, total_bytes, 1 , fp);
if (count <= 0){
cerr << "read file error" << endl;
fclose(fp);
free(pixels);
return 0;
}
glGetIntegerv(GL_MAX_TEXTURE_SIZE, &Max);
{
if (!power_of_tw(width) || !power_of_tw(height) || width>Max || height>Max){
const GLint width_zoom = 64;
const GLint height_zoom = 64;
GLint new_line_bytes, new_total_bytes;
GLubyte *pixels_zoom = NULL;
new_line_bytes = width_zoom * 3;
while (new_line_bytes % 4 != 0)
++new_line_bytes;
new_total_bytes = new_line_bytes * height_zoom;
pixels_zoom = (GLubyte *)malloc(new_total_bytes);
if (pixels_zoom == NULL){
cerr << "malloc wrong pixels_zoom" << endl;
free(pixels);
fclose(fp);
return 0;
}
gluScaleImage(GL_RGB, \
width, height, GL_UNSIGNED_BYTE, pixels, \
width_zoom, height_zoom, GL_UNSIGNED_BYTE, pixels_zoom);
free(pixels);
pixels = pixels_zoom;
width = width_zoom;
height = height_zoom;
}
}

// ·ÖÅäÒ»¸öÐÂµÄÎÆÀí±àºÅ
glGenTextures(1, &texture_ID);
cout<<texture_ID;
if (texture_ID == 0)
{
cout << "generate texture_ID failed" << endl;
free(pixels);
fclose(fp);
return 0;
}

glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture_ID);
glBindTexture(GL_TEXTURE_2D, texture_ID);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
glBindTexture(GL_TEXTURE_2D, last_texture_ID);

free(pixels);
return texture_ID;
}

bool power_of_tw(GLuint num)
{
if (num <= 0)
return false;
return (num & (num - 1)) == 0;
}

*/


void ReadMtl(string &cd, string mtlfile, map<string, Material> &mat)
{
	ifstream in;
	string line, word, ptname, ntname, fname;
	unsigned map = 0;
	bool hasmap = false;
	float ambient[3] = { 0.0, 0.0, 0.0 };
	float diffuse[3] = { 0.0, 0.0, 0.0 };
	float specular[3] = { 0.0, 0.0, 0.0 };
	float emission[3] = { 0.0, 0.0, 0.0 };

	in.open((cd + "\\" + mtlfile).c_str());
	if (!in)
	{
		cout << "Read mtl error !" << endl;
		return;
	}
	while (getline(in, line))
	{
		if (line.size() == 0 || line[0] == '#') continue;
		istringstream is(line);
		is >> word;
		if (word == "newmtl")
		{
			is >> ntname;
			if (!ptname.empty())
			{
				if (hasmap)
				{
					mat.insert(make_pair(ptname, Material(ambient, diffuse, specular, emission, map)));
				}
				else
				{
					mat.insert(make_pair(ptname, Material(ambient, diffuse, specular, emission, 0)));
				}
			}
			ptname = ntname;
			hasmap = false;
		}
		else if (word == "Ka")
		{
			is >> ambient[0] >> ambient[1] >> ambient[2];
		}
		else if (word == "Kd")
		{
			is >> diffuse[0] >> diffuse[1] >> diffuse[2];
		}
		else if (word == "Ks")
		{
			is >> specular[0] >> specular[1] >> specular[2];
		}
		else if (word == "Ke")
		{
			is >> emission[0] >> emission[1] >> emission[2];
		}
		else if (word == "map_Ka")
		{
			is >> fname;
			map = load_texture_LLC((cd + "\\" + fname).c_str());
			hasmap = true;
		}
	}
	if (!ntname.empty())
	{
		if (hasmap)
		{
			mat.insert(make_pair(ptname, Material(ambient, diffuse, specular, emission, map)));
		}
		else
		{
			mat.insert(make_pair(ptname, Material(ambient, diffuse, specular, emission, 0)));
		}
	}
	in.close();
}

void ReadObj(string &cd, string file, map<string, Object> &m, set<string> &n, map<string, Material> &matname)
{
	ifstream in;
	vector<VERTEX> vertexs;
	vector< pair<float, float> > texcoords;
	vector<VERTEX> normals;
	vector<int> faces;

	int row = 0, col = 0;
	string line, word, goname, mtlname;
	char Buffer[MAX_PATH];

	if (file.find(":") != string::npos)
	{
		cd = string(file.begin(), file.begin() + file.rfind("\\"));
	}
	else if (startswith(file, string(".\\")))
	{
		GetCurrentDirectoryA(MAX_PATH, Buffer);
		cd = Buffer + string(file.begin() + 1, file.begin() + file.rfind("\\"));
	}
	else if (startswith(file, string("..\\")))
	{
		GetCurrentDirectoryA(MAX_PATH, Buffer);
		cd = Buffer;
		cd = string(cd.begin(), cd.begin() + cd.rfind("\\"));
		cd = cd + string(file.begin() + 2, file.begin() + file.rfind("\\"));
		cout << cd << endl;
	}
	else
	{
		GetCurrentDirectoryA(MAX_PATH, Buffer);
		if (file.rfind("\\") != string::npos)
		{
			cd = Buffer + string("\\") + string(file.begin(), file.begin() + file.rfind("\\"));
		}
		else
		{
			cd = Buffer;
		}
	}

	in.open(file.c_str());
	if (!in)
	{
		cout << "Read obj error !" << endl;
		exit(0);
	}
	while (getline(in, line))
	{
		if (line.size() == 0 || line[0] == '#') continue;
		istringstream is(line);
		is >> word;
		if (word == "v")
		{
			VERTEX p;
			is >> p.x >> p.y >> p.z;
			vertexs.push_back(p);
		}
		else if (word == "vt")
		{
			pair<float, float> p;
			is >> p.first >> p.second;
			texcoords.push_back(p);
		}
		else if (word == "vn")
		{
			VERTEX p;
			is >> p.x >> p.y >> p.z;
			normals.push_back(p);
		}
		else if (word == "o" || word == "g")
		{
			if (!goname.empty() && !faces.empty())
			{
				Object obj(vertexs.begin(), vertexs.end(), texcoords.begin(), texcoords.end(), normals.begin(), normals.end(), faces.begin(), faces.end(), row, col, mtlname);
				while (n.count(goname) != 0)
				{
					goname.push_back('0');
				}
				m.insert(make_pair(goname, obj));
				n.insert(goname);
				faces.clear();
			}
			is >> goname;
		}
		else if (word == "f")
		{
			int r = 0, c = 0;
			while (is >> word)
			{
				c = count(word.begin(), word.end(), '/');
				if (c == 0)
				{
					faces.push_back(atoi(word.c_str()));
				}
				else if (c == 1)
				{
					faces.push_back(atoi(string(word.begin(), word.begin() + word.find("/")).c_str()));
					faces.push_back(atoi(string(word.begin() + word.find("/") + 1, word.end()).c_str()));
				}
				else if (c == 2)
				{
					int a = word.find("/");
					int b = word.find("/", a + 1);
					faces.push_back(atoi(string(word.begin(), word.begin() + a).c_str()));
					faces.push_back(atoi(string(word.begin() + a + 1, word.begin() + b).c_str()));
					faces.push_back(atoi(string(word.begin() + b + 1, word.end()).c_str()));
				}
				++r;
			}
			row = r;
			col = c + 1;
		}
		else if (word == "mtllib")
		{
			is >> word;
			ReadMtl(cd, word, matname);
		}
		else if (word == "usemtl")
		{
			is >> mtlname;
		}
	}
	if (!goname.empty() && !faces.empty())
	{
		Object obj(vertexs.begin(), vertexs.end(), texcoords.begin(), texcoords.end(), normals.begin(), normals.end(), faces.begin(), faces.end(), row, col, mtlname);
		while (n.count(goname) != 0)
		{
			goname.push_back('0');
		}
		m.insert(make_pair(goname, obj));
		n.insert(goname);
		faces.clear();
	}
	in.close();
}
