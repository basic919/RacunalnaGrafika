#include <cstdio>
#include <glm/glm.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <GL/glut.h>
using namespace std;


class Vertex
{
public:
    Vertex(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}

    glm::vec3 get() { return glm::vec3(x, y, z); }
    double x, y, z;
};

class MyPolygon
{
public:
    MyPolygon(Vertex _v1, Vertex _v2, Vertex _v3) : v1(_v1), v2(_v2), v3(_v3) {}

    Vertex v1, v2, v3;
    glm::vec3 normal()
    {
        glm::vec3 edge1 = v2.get() - v1.get();
        glm::vec3 edge2 = v3.get() - v1.get();
        glm::vec3 normal = glm::cross(edge1, edge2);
        return glm::normalize(normal);
    }
};

class Material
{
public:
    Material(string _name) : name(_name) {}

    string name;  //naziv
    glm::vec3 Ka; //ambient
    glm::vec3 Kd; //difuse
    glm::vec3 Ks; //specular
    double d;     //roughness
};

class Object3D
{
public:
    Object3D(string _name) : name(_name) {}

    string name;
    vector<MyPolygon> polygons;
    Material* material;
};


class MagicParticle
{
public:
    MagicParticle(Vertex _position, glm::vec3 _color, double _size) : position(_position), color(_color), size(_size)
    {
        lifetime = 1.0;
    };

    Vertex position;
    glm::vec3 color;
    double size;
    double lifetime;

    void render()
    {
        glColor3f(color.x, color.y, color.z);
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glutSolidSphere(size, 3, 3);
        glPopMatrix();
    }

};

//  Globalne varijable

vector<Vertex> vertices;
vector<Vertex> normals;
vector<glm::vec2> textures;
vector<Object3D> objects;
vector<Material*> materials;
vector<MagicParticle> magic;

bool runParticles = false;


static const string sceneFilePath = "Staff.obj";
static const string materialFilePath = "Staff.mtl";

GLuint window;
GLuint width = 950, height = 750;

Vertex O(-8.0, -6.0, -5.0);
Vertex I(3.0, 7.0, 3.0);

glm::vec3 light = glm::vec3(1.0, 1.0, 1.0);
glm::vec3 ambientLight = glm::vec3(0.5, 0.5, 0.5);

Vertex magic_starting_point = Vertex(0.0, 0.0, 0.0);



void myDisplay();
void myIdle();
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
void readSceneFile(string filePath);
void readMaterialFile(string filePath);
void myReshape(int width, int height);
void updatePerspective();
void renderObjects();
void renderObject(Object3D object);
void renderMagic();
void magicParticles();
double clamp(double x, double lower, double upper);
double getRand();


int main(int argc, char** argv)
{
    srand(time(NULL));

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(400, 200);
    glutInit(&argc, argv);

    window = glutCreateWindow("Elder Wand");
    glutDisplayFunc(myDisplay);
    glutKeyboardFunc(myKeyboard);
    glutReshapeFunc(myReshape);

    readMaterialFile(materialFilePath);
    readSceneFile(sceneFilePath);

    glutIdleFunc(myIdle);
    glutMainLoop();
    return 0;
}


void myDisplay()
{
    glClearColor(+0.00f, +0.00f, +0.0f, +1.0f);
    //glClearColor(+0.07f, +0.041f, +0.12f, +1.0f);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderObjects();

    renderMagic();

    glutSwapBuffers();
}

int current_time = 0;
int previous_time = 0;

void myIdle()
{
    current_time = glutGet(GLUT_ELAPSED_TIME);
    int diff = current_time - previous_time;
    if (diff > 20)
    {
        magicParticles();

        myDisplay();
        previous_time = current_time;
    }
}

void myReshape(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, width, height);
    updatePerspective();
}

void updatePerspective()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)width / height, 0.5, 80.0); // kut pogleda, x/y, prednja i straznja ravnina odsjecanja
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(O.x, O.y, O.z, 0.0, 1.5, 0.0, 1.0, 1.0, 0.0); // O x,y,z; glediste x,y,z; up vektor x,y,z
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY)
{
    switch (theKey)
    {
    case ' ':
        runParticles = !runParticles;
        break;

    case 'l':
        O.x = O.x + 0.1;
        break;

    case 'k':
        O.x = O.x - 0.1;
        break;

    case 'i':
        O.y = O.y + 0.1;
        break;

    case 'o':
        O.y = O.y - 0.1;
        break;

    case 'n':
        O.z = O.z + 0.1;
        break;

    case 'm':
        O.z = O.z - 0.1;
        break;

    case 'r':
        O.x = -8.0;
        O.y = -6.0;
        O.z = -5.0;
        break;

    default:
        break;
    }
    cout << "O: " << O.x << " " << O.y << " " << O.z << endl;
    updatePerspective();
    glutPostRedisplay();
}

// random double in range [-1, 1]
double getRand()
{
    return (double)rand() / RAND_MAX;
}

void magicParticles()
{
    // create new particle
    Vertex position = magic_starting_point;
    double size = 0.05;
    
    if (runParticles) {
        for (int parts = 0; parts < 20; parts++) {
            float clr = abs(getRand());
            glm::vec3 color = glm::vec3(0.9, 0.95, 1.0);
            if (clr < 0.45)
            {
                color = glm::vec3(0.051, 0.808, 0.859);
            }
            else {
                if (clr < 0.55) {
                    color = glm::vec3(0.675, 0.941, 0.998);
                }
                else {
                    if (clr < 0.75) {
                        color = glm::vec3(0.114, 0.510, 0.878);
                    }
                }
            }

            MagicParticle fp(position, color, size);
            magic.push_back(fp);
        }
    }

    //cout << magic.size() << "\n";
    // edit existing particles
    for (int i = magic.size() - 1; i >= 0; i--)
    {
        // transition
        Vertex current_position = magic.at(i).position;
        double x = -0.025 + getRand() * 0.05;
        double z = -0.025 + getRand() * 0.05;

        //double cosphi = x / sqrt(pow(x, 2) + pow(z, 2));
        //double phi = acos(cosphi);
        //x -= 0.01 * sin(phi);
        //z += 0.01 * cosphi;

        magic.at(i).position = Vertex(current_position.x + x, clamp(current_position.y + 0.1*abs(getRand()), 0.1, 14.5), current_position.z + z);

        // color fade

        float life_left = magic.at(i).lifetime;

        if (life_left < 0.55) {
            magic.at(i).color.x = min(magic.at(i).color.x + 0.015, 0.742);
            magic.at(i).color.y = min(magic.at(i).color.y + 0.015, 0.82);
            magic.at(i).color.z = min(magic.at(i).color.z + 0.02, 1.0);
        }

        // death
        magic.at(i).lifetime -= 0.005;
        if (magic.at(i).lifetime <= 0.0)
        {
            magic.erase(magic.begin() + i);
        }
    }
}

void readSceneFile(string filePath)
{
    ifstream file(filePath);
    string line;
    string previous_line = "NULL";
    if (!file)
    {
        cerr << "Problem prilikom ucitavanja datoteke " << filePath << endl;
        exit(1);
    }
    while (true)
    {
        if (line.compare(0, 1, "") == 0 && previous_line.compare(0, 1, "o") == 0)
        {
            break;
        }
        if (previous_line.compare("NULL") == 0)
        {
            if (!getline(file, line))
            {
                break;
            }
        }
        else
        {
            line = previous_line;
        }
        //linija pocinje s o => novi objekt
        if (line.compare(0, 1, "o") == 0)
        {
            string name;
            char c;
            istringstream iss(line);
            iss >> c >> name;
            Object3D object(name);
            vector<MyPolygon> obj_polygons;
            while (getline(file, line))
            {
                // linija pocinje s vn -> normala vrha
                if (line.compare(0, 2, "vn") == 0)
                {
                    double x, y, z;
                    string vn;
                    istringstream iss(line);
                    iss >> vn >> x >> y >> z;
                    Vertex v(x, y, z);
                    normals.push_back(v);
                }
                // linija pocinje s vt -> textura vrha
                else if (line.compare(0, 2, "vt") == 0)
                {
                    double x, y;
                    string vt;
                    istringstream iss(line);
                    iss >> vt >> x >> y;
                    glm::vec2 v(x, y);
                    textures.push_back(v);
                }
                // linija pocinje s v => vrh
                else if (line.compare(0, 1, "v") == 0)
                {
                    double x, y, z;
                    char c;
                    istringstream iss(line);
                    iss >> c >> x >> y >> z;
                    Vertex v(x, y, z);
                    vertices.push_back(v);
                }
                // linija pocinje s f => poligon
                else if (line.compare(0, 1, "f") == 0)
                {
                    string V1, V2, V3;
                    char c;
                    istringstream iss(line);
                    iss >> c >> V1 >> V2 >> V3;
                    int indexV1 = stoi(V1.substr(0, V1.find("//")));
                    int indexV2 = stoi(V2.substr(0, V2.find("//")));
                    int indexV3 = stoi(V3.substr(0, V3.find("//")));
                    Vertex v1 = vertices[indexV1 - 1];
                    Vertex v2 = vertices[indexV2 - 1];
                    Vertex v3 = vertices[indexV3 - 1];
                    MyPolygon p(v1, v2, v3);
                    obj_polygons.push_back(p);
                }
                // linija pocinje s usemtl => naziv materijala
                else if (line.compare(0, 6, "usemtl") == 0)
                {
                    string name;
                    string usemtl;
                    istringstream iss(line);
                    iss >> usemtl >> name;
                    auto it = find_if(materials.begin(), materials.end(), [&name](const Material* obj) { return obj->name == name; });

                    if (it != materials.end())
                    {
                        // found element. it is an iterator to the first matching element.
                        // if you really need the index, you can also get it:
                        auto index = distance(materials.begin(), it);
                        object.material = materials[index];
                    }
                }
                // linija pocinje sa o => pocetak novog objekta
                else if (line.compare(0, 1, "o") == 0)
                {
                    previous_line = line;
                    break;
                }
                else
                {
                    continue;
                }
            }
            object.polygons = obj_polygons;
            objects.push_back(object);
        }
    }
}

void readMaterialFile(string filePath)
{
    ifstream file(filePath);
    string line;
    if (!file)
    {
        cerr << "Problem prilikom ucitavanja datoteke " << filePath << endl;
        exit(1);
    }
    while (getline(file, line))
    {
        //linija pocinje s newmtl => novi materijal
        if (line.compare(0, 6, "newmtl") == 0)
        {
            string name;
            string newmtl;
            istringstream iss(line);
            iss >> newmtl >> name;
            Material* material = new Material(name);
            while (getline(file, line))
            {
                // linija pocinje s Ka => ambijentalna komponenta
                if (line.compare(0, 2, "Ka") == 0)
                {
                    double x, y, z;
                    string Ka;
                    istringstream iss(line);
                    iss >> Ka >> x >> y >> z;
                    glm::vec3 ka(x, y, z);
                    material->Ka = ka;
                }
                // linija pocinje s Kd => difuzna komponenta
                else if (line.compare(0, 2, "Kd") == 0)
                {
                    double x, y, z;
                    string Kd;
                    istringstream iss(line);
                    iss >> Kd >> x >> y >> z;
                    glm::vec3 kd(x, y, z);
                    material->Kd = kd;
                }
                // linija pocinje s Ks => spekularna komponenta
                else if (line.compare(0, 2, "Ks") == 0)
                {
                    double x, y, z;
                    string Ks;
                    istringstream iss(line);
                    iss >> Ks >> x >> y >> z;
                    glm::vec3 ks(x, y, z);
                    material->Ks = ks;
                }
                // linija pocinje s d => indeks hrapavosti
                else if (line.compare(0, 1, "d") == 0)
                {
                    double n;
                    char d;
                    istringstream iss(line);
                    iss >> d >> n;
                    material->d = n;
                }
                // linija pocinje s illum => kraj opisa objekta
                else if (line.compare(0, 5, "illum") == 0)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
            materials.push_back(material);
        }
    }
}

void renderObjects()
{
    for (auto& obj : objects)
    {
        renderObject(obj);
    }
}

double clamp(double x, double lower, double upper)
{
    return min(upper, max(x, lower));
}

glm::vec3 diffuse_specular(glm::vec3 point, glm::vec3 centroid, glm::vec3 kd, glm::vec3 ks, double n)
{
    glm::vec3 color = glm::vec3(0.0, 0.0, 0.0);

    glm::vec3 L = glm::vec3(I.x - point.x, I.y - point.y, I.z - point.z);
    L = glm::normalize(L);
    glm::vec3 N = glm::vec3(point.x - centroid.x, point.y - centroid.y, point.z - centroid.z);
    N = glm::normalize(N);
    double LN = glm::dot(L, N);

    // Difuzno (Ii * kd * (L*N))
    double red = light.x * kd.x * LN;
    double green = light.y * kd.y * LN;
    double blue = light.z * kd.z * LN;
    color = color + glm::vec3(red, green, blue);

    // R = N * (2 * L*N) - L
    glm::vec3 R = glm::vec3(2 * LN * N.x, 2 * LN * N.y, 2 * LN * N.z) - L;
    R = glm::normalize(R);
    glm::vec3 V = glm::vec3(O.x - point.x, O.y - point.y, O.z - point.z);
    V = glm::normalize(V);
    double RV = glm::dot(R, V);

    // Spekularno (Ii * ks * (R*V)^n)
    red = light.x * ks.x * pow(RV, n);
    green = light.y * ks.y * pow(RV, n);
    blue = light.z * ks.z * pow(RV, n);
    color = color + glm::vec3(red, green, blue);

    return color;
}

void renderObject(Object3D object)
{
    for (auto& poly : object.polygons)
    {
        glBegin(GL_TRIANGLES);

        // Ambijentno (Ia * ka)
        glm::vec3 ka = object.material->Ka;
        double red = ambientLight.x * ka.x;
        double green = ambientLight.y * ka.y;
        double blue = ambientLight.z * ka.z;
        glm::vec3 ambient = glm::vec3(red, green, blue);

        double x = (poly.v1.x + poly.v2.x + poly.v3.x) / 3.0;
        double y = (poly.v1.y + poly.v2.y + poly.v3.y) / 3.0;
        double z = (poly.v1.z + poly.v2.z + poly.v3.z) / 3.0;
        glm::vec3 centroid = glm::vec3(x, y, z);
        glm::vec3 kd = object.material->Kd;
        glm::vec3 ks = object.material->Ks;
        double n = object.material->d;

        glm::vec3 point_1 = glm::vec3(poly.v1.x, poly.v1.y, poly.v1.z);
        glm::vec3 diffuse_1 = diffuse_specular(point_1, centroid, kd, ks, n);
        glm::vec3 color_1 = ambient + diffuse_1;
        glColor3f(clamp(color_1.x, 0.0, 1.0), clamp(color_1.y, 0.0, 1.0), clamp(color_1.z, 0.0, 1.0));
        glVertex3f(poly.v1.x, poly.v1.y, poly.v1.z);

        glm::vec3 point_2 = glm::vec3(poly.v2.x, poly.v2.y, poly.v2.z);
        glm::vec3 diffuse_2 = diffuse_specular(point_2, centroid, kd, ks, n);
        glm::vec3 color_2 = ambient + diffuse_2;
        glColor3f(clamp(color_2.x, 0.0, 1.0), clamp(color_2.y, 0.0, 1.0), clamp(color_2.z, 0.0, 1.0));
        glVertex3f(poly.v2.x, poly.v2.y, poly.v2.z);

        glm::vec3 point_3 = glm::vec3(poly.v3.x, poly.v3.y, poly.v3.z);
        glm::vec3 diffuse_3 = diffuse_specular(point_3, centroid, kd, ks, n);
        glm::vec3 color_3 = ambient + diffuse_3;
        glColor3f(clamp(color_3.x, 0.0, 1.0), clamp(color_3.y, 0.0, 1.0), clamp(color_3.z, 0.0, 1.0));
        glVertex3f(poly.v3.x, poly.v3.y, poly.v3.z);
        glEnd();
    }
}

void renderMagic()
{
    for (auto& particle : magic)
    {
        particle.render();
    }
}