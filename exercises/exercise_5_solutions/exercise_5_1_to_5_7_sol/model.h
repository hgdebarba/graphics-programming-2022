// modified version of https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/model.h
#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"
#include "shader.h"
// NEW! our models are stored in a specific 3D mesh format (i.e. no longer in a header file)
//  objloader is used to parse those files
#include "objloader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;


class Model
{
public:
    /*  Model Data */
    std::vector<Mesh> meshes;
    string directory;

    /*  Functions   */
    // constructor, expects a filepath to a 3D model.
    Model(string const &path)
    {
        loadModel(path);
    }

    Model(std::vector<string> const &paths)
    {
        for(auto path : paths)
            loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw()
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw();
    }

private:
    /*  Functions   */
    // loads a model
    void loadModel(string const &path)
    {

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        loadOBJ(path.c_str(), vertices, uvs, normals);
        meshes.push_back(processMesh(vertices, uvs, normals));

    }


    Mesh processMesh(const std::vector<glm::vec3> & inVertices,
                     const std::vector<glm::vec2> & inUvs,
                     const std::vector<glm::vec3> & inNormals)
    {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // Walk through each of the mesh's vertices
        for(unsigned int i = 0; i < inVertices.size(); i++)
        {
            Vertex vertex;
            glm::vec3 vector;
            // positions
            vertex.Position = inVertices[i];
            // normals
            vertex.Normal = inNormals[i];
            // texture coordinates
            vertex.TexCoords = i < inUvs.size() ? inUvs[i] : glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
            indices.push_back(i);
        }

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices);//, textures);
    }

};

#endif