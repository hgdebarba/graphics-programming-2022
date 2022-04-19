//
// Created by henrique on 04/11/2021.
//

#ifndef ITU_GRAPHICS_PROGRAMMING_FRAME_BUFFER_H
#define ITU_GRAPHICS_PROGRAMMING_FRAME_BUFFER_H


template<class T>
class FrameBuffer {
public:
    unsigned int W, H;
    T *buffer;

    FrameBuffer(unsigned int width, unsigned int height) : W(width), H(height) {
        buffer = new T[W * H];
    }

    ~FrameBuffer() { delete[] buffer; } // clean our memory

    void clearBuffer(T value) {
        int size = W * H;
        for (int i = 0; i < size; i++)
            buffer[i] = value;
    }

    void paintAt(unsigned int x, unsigned int y, T value) {
        assert(x < W && y < H); // ensure valid position, crash if not (sooo dramatic!)
        buffer[x + y * W] = value;
    }

    T valueAt(unsigned int x, unsigned int y) {
        assert(x < W && y < H);
        return buffer[x + y * W];
    }

};


#endif //ITU_GRAPHICS_PROGRAMMING_FRAME_BUFFER_H
