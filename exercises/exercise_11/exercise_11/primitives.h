//
// Created by Henrique on 9/17/2019.
//

#ifndef GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H
#define GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H


struct Primitives {
private:
    Primitives() = default;

public:
    // the getInstance and deleted functions below makes this a singleton
    static Primitives& getInstance()
    {
        static Primitives instance;
        return instance;
    }
    Primitives(Primitives const&)      = delete;
    void operator=(Primitives const&)  = delete;


    static void makeCube(float side, std::vector<glm::vec3>& positions,
                         std::vector<glm::vec3>& normals,
                         std::vector<glm::vec2>& uvs,
                         std::vector<glm::vec4>& colors){
        using namespace glm;
        using namespace std;

        float length = side * .5f;

        //    v7----- v6
        //   /|      /|
        //  v3------v2|
        //  | |     | |
        //  | |v4---|-|v5
        //  |/      |/
        //  v0------v1

        vec3 p[] = {
                vec3{-length, -length, length},
                vec3{length, -length, length},
                vec3{length, length, length},
                vec3{-length, length, length},

                vec3{-length, -length, -length},
                vec3{length, -length, -length},
                vec3{length, length, -length},
                vec3{-length, length, -length}
        };

        positions = vector<vec3>({p[0],p[1],p[2],  p[0],p[2],p[3],
                                p[1],p[5],p[6],  p[1],p[6],p[2],
                                p[5],p[4],p[7],  p[5],p[7],p[6],
                                p[4],p[0],p[3],  p[4],p[3],p[7],
                                p[3],p[2],p[6],  p[3],p[6],p[7],
                                p[1],p[0],p[4],  p[1],p[4],p[5],
                               });

        vec4 c[] = {

                vec4{.9, .1, .1, 1},
                vec4{.1, .9, .1, 1},
                vec4{.1, .1, .9, 1},
                vec4{.9, .9, .1, 1},

                vec4{.9, .1, .9, 1},
                vec4{.1, .9, .9, 1},
                vec4{.1, .1, .1, 1},
                vec4{.9, .9, .9, 1}
        };

        colors = vector<vec4>({c[0],c[1],c[2],  c[0],c[2],c[3],
                                  c[1],c[5],c[6],  c[1],c[6],c[2],
                                  c[5],c[4],c[7],  c[5],c[7],c[6],
                                  c[4],c[0],c[3],  c[4],c[3],c[7],
                                  c[3],c[2],c[6],  c[3],c[6],c[7],
                                  c[1],c[0],c[4],  c[1],c[4],c[5]
                                 });

        vec2 u[] = {
                vec4(0,0,0,0),
                vec4(1,0,0,0),
                vec4(1,1,0,0),
                vec4(0,1,0,0)
        };
        uvs = vector<vec2>({ u[0],u[1],u[2],  u[0],u[2],u[3],
                           u[0],u[1],u[2],  u[0],u[2],u[3],
                           u[0],u[1],u[2],  u[0],u[2],u[3],
                           u[0],u[1],u[2],  u[0],u[2],u[3],
                           u[0],u[1],u[2],  u[0],u[2],u[3],
                           u[0],u[1],u[2],  u[0],u[2],u[3],
                         });
        vec3 n[] = {
                vec3{0, 0, 1},
                vec3{1, 0, 0},
                vec3{0, 0, -1},
                vec3{-1, 0, 0},

                vec3{0, 1, 0},
                vec3{0, -1, 0}
        };
        normals = vector<vec3>({
                                     n[0],n[0],n[0],  n[0],n[0],n[0],
                                     n[1],n[1],n[1],  n[1],n[1],n[1],
                                     n[2],n[2],n[2],  n[2],n[2],n[2],
                                     n[3],n[3],n[3],  n[3],n[3],n[3],
                                     n[4],n[4],n[4],  n[4],n[4],n[4],
                                     n[5],n[5],n[5],  n[5],n[5],n[5]
                             });
    }


};


#endif //GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H
