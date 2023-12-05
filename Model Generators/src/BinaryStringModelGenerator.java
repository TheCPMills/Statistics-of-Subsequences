import java.util.*;
import org.javatuples.*;
import glm.*;

public class BinaryStringModelGenerator {
    public static Quartet<List<Face>, vec3, List<vec3>, Integer> modelZero(int bit, vec3 position, float scaleFactor, List<vec3> vertexList, int nextVertexIndex) {
        List<vec3> zeroVertices = new ArrayList<vec3>() {
            {
                add(new vec3(0.054f, 0.000f, 0.019f)); // 0
                add(new vec3(0.054f, 0.000f, 0.000f)); // 1
                add(new vec3(0.079f, 0.000f, 0.004f)); // 2
                add(new vec3(0.067f, 0.000f, 0.021f)); // 3
                add(new vec3(0.078f, 0.000f, 0.028f)); // 4
                add(new vec3(0.093f, 0.000f, 0.014f)); // 5
                add(new vec3(0.104f, 0.000f, 0.031f)); // 6
                add(new vec3(0.084f, 0.000f, 0.039f)); // 7
                add(new vec3(0.087f, 0.000f, 0.053f)); // 8
                add(new vec3(0.108f, 0.000f, 0.053f)); // 9
                add(new vec3(0.108f, 0.000f, 0.132f)); // 10
                add(new vec3(0.087f, 0.000f, 0.132f)); // 11
                add(new vec3(0.104f, 0.000f, 0.154f)); // 12
                add(new vec3(0.084f, 0.000f, 0.146f)); // 13
                add(new vec3(0.078f, 0.000f, 0.157f)); // 14
                add(new vec3(0.093f, 0.000f, 0.171f)); // 15
                add(new vec3(0.079f, 0.000f, 0.181f)); // 16
                add(new vec3(0.067f, 0.000f, 0.164f)); // 17
                add(new vec3(0.054f, 0.000f, 0.166f)); // 18
                add(new vec3(0.054f, 0.000f, 0.185f)); // 19
                add(new vec3(0.029f, 0.000f, 0.181f)); // 20
                add(new vec3(0.041f, 0.000f, 0.164f)); // 21
                add(new vec3(0.030f, 0.000f, 0.157f)); // 22
                add(new vec3(0.015f, 0.000f, 0.171f)); // 23
                add(new vec3(0.004f, 0.000f, 0.154f)); // 24
                add(new vec3(0.024f, 0.000f, 0.146f)); // 25
                add(new vec3(0.021f, 0.000f, 0.132f)); // 26
                add(new vec3(0.000f, 0.000f, 0.132f)); // 27
                add(new vec3(0.000f, 0.000f, 0.053f)); // 28
                add(new vec3(0.021f, 0.000f, 0.053f)); // 29
                add(new vec3(0.004f, 0.000f, 0.031f)); // 30
                add(new vec3(0.024f, 0.000f, 0.039f)); // 31
                add(new vec3(0.030f, 0.000f, 0.028f)); // 32
                add(new vec3(0.015f, 0.000f, 0.014f)); // 33
                add(new vec3(0.029f, 0.000f, 0.004f)); // 34
                add(new vec3(0.041f, 0.000f, 0.021f)); // 35
            }
        };

        int vertexIndices[] = {
            0, 1, 2, 3,
            3, 2, 5, 4,
            4, 5, 6, 7,
            7, 6, 9, 8,
            8, 9, 10, 11,
            11, 10, 12, 13,
            13, 12, 15, 14,
            14, 15, 16, 17,
            17, 16, 19, 18,
            18, 19, 20, 21,
            21, 20, 23, 22,
            22, 23, 24, 25,
            25, 24, 27, 26,
            28, 29, 26, 27,
            29, 28, 30, 31,
            31, 30, 33, 32,
            32, 33, 34, 35,
            35, 34, 1, 0
        };

        for (int i = 0; i < zeroVertices.size(); i++) {
            vec3 vertex = zeroVertices.get(i);
            zeroVertices.set(i, vertex.scale(scaleFactor).add(position));
        }
        vertexList.addAll(zeroVertices);
        
        List<Face> faces = new ArrayList<Face>();
        for (int i = 0; i < vertexIndices.length; i += 4) {
            int faceVertexIndices[] = {vertexIndices[i] + nextVertexIndex, vertexIndices[i + 1] + nextVertexIndex, vertexIndices[i + 2] + nextVertexIndex, vertexIndices[i + 3] + nextVertexIndex};
            Face face = new Face(faceVertexIndices, new int[]{0, 0, 0, 0}, 1);
            faces.add(face);
        }

        position = position.add(new vec3(0.108f + 0.0125f, 0.0f, 0.0f).scale(scaleFactor));

        return new Quartet<>(faces, position, vertexList, nextVertexIndex + zeroVertices.size());
    }

    public static Quartet<List<Face>, vec3, List<vec3>, Integer> modelOne(int bit, vec3 position, float scaleFactor, List<vec3> vertexList, int nextVertexIndex) {
        List<vec3> oneVertices = new ArrayList<vec3>() {
            {
                add(new vec3(0.030f, 0.000f, 0.002f)); // 0
                add(new vec3(0.052f, 0.000f, 0.002f)); // 1
                add(new vec3(0.052f, 0.000f, 0.183f)); // 2
                add(new vec3(0.030f, 0.000f, 0.183f)); // 3
                add(new vec3(0.000f, 0.000f, 0.166f)); // 4
                add(new vec3(0.000f, 0.000f, 0.145f)); // 5
                add(new vec3(0.030f, 0.000f, 0.159f)); // 6
            }
        };

        int vertexIndices[] = {
            0, 1, 2, 3,
            3, 4, 5, 6
        };

        for (int i = 0; i < oneVertices.size(); i++) {
            vec3 vertex = oneVertices.get(i);
            oneVertices.set(i, vertex.scale(scaleFactor).add(position));
        }
        vertexList.addAll(oneVertices);
        
        List<Face> faces = new ArrayList<Face>();
        for (int i = 0; i < vertexIndices.length; i += 4) {
            int faceVertexIndices[] = {vertexIndices[i] + nextVertexIndex, vertexIndices[i + 1] + nextVertexIndex, vertexIndices[i + 2] + nextVertexIndex, vertexIndices[i + 3] + nextVertexIndex};
            Face face = new Face(faceVertexIndices, new int[]{0, 0, 0, 0}, 1);
            faces.add(face);
        }

        position = position.add(new vec3(0.052f + 0.025f, 0.0f, 0.0f).scale(scaleFactor));

        return new Quartet<>(faces, position, vertexList, nextVertexIndex + oneVertices.size());
    }
}
