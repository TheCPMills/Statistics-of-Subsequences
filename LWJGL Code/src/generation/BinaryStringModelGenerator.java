package generation;

import java.io.*;
import java.util.*;
import org.javatuples.*;
import glm.*;

public class BinaryStringModelGenerator {
    public static void main(String[] args) throws Exception {
        int n = 3;
        int bits = 4;

        generate(n, bits);
    }

    private static void generate(int n, int bits) throws Exception {
        FileWriter objWriter = new FileWriter(new File(System.getProperty("user.dir") + "/res/models/binary_string.obj"));
        genObj(n, bits, objWriter);
        objWriter.close();

        FileWriter mtlWriter = new FileWriter(new File(System.getProperty("user.dir") + "/res/models/binary_string.mtl"));
        genMtl(n, bits, mtlWriter);
        mtlWriter.close();
    }

    private static void genObj(int n, int bits, FileWriter objWriter) throws Exception {
        objWriter.write("mtllib binary_string.mtl\n\n");

        String nInBinary = Integer.toBinaryString(n);
        while(nInBinary.length() < bits) {
            nInBinary = "0" + nInBinary;
        }

        int nextVertexIndex = 0;
        List<vec3> vertices = new ArrayList<>();
        vec2 uv = new vec2(0.0f, 0.0f);
        vec3 normal = new vec3(0.0f, 1.0f, 0.0f);
        BinaryStringObjectGroup object = new BinaryStringObjectGroup(nInBinary + "[binary_string]", new ArrayList<Face>(), "binary_string");

        for(int bit = 0; bit < nInBinary.length(); bit++) {
            int currentBit = nInBinary.charAt(bit) - '0';

            Triplet<List<Face>, List<vec3>, Integer> result;
            if (currentBit == 0) {
                result = modelZero(bit, vertices, nextVertexIndex);
            } else {
                result = modelOne(bit, vertices, nextVertexIndex);
            }

            List<Face> faces = result.getValue0();
            vertices = result.getValue1();
            nextVertexIndex = result.getValue2();

            object.addFaces(faces);
        }

        // write vertices
        for(vec3 vertex : vertices) {
            objWriter.write("v " + vertex.x + " " + vertex.y + " " + vertex.z + "\n");
        }
        objWriter.write("\n");

        // write uv
        objWriter.write("vt " + uv.x + " " + uv.y + "\n\n");

        // write normal
        objWriter.write("vn " + normal.x + " " + normal.y + " " + normal.z + "\n\n");

        // write object
        objWriter.write(object.objectString() + "\n");
    }

    private static void genMtl(int n, int m, FileWriter mtlWriter) throws Exception {
        mtlWriter.write("newmtl binary_string\n");
        mtlWriter.write("Ka 0.0 0.0 0.0\n");
        mtlWriter.write("Kd 0.0 0.0 0.0\n");
        mtlWriter.write("Ks 0.0 0.0 0.0\n");
        mtlWriter.write("Ke 0.0 0.0 0.0\n");
        mtlWriter.write("Ns 10.0\n");
        mtlWriter.write("Ni 1.45\n");
        mtlWriter.write("d 1.0\n");
        mtlWriter.write("illum 2\n");

        mtlWriter.write("\n");
    }

    private static Triplet<List<Face>, List<vec3>, Integer> modelZero(int bit, List<vec3> vertexList, int nextVertexIndex) {
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
            zeroVertices.set(i, vertex.add(new vec3(bit * 0.118f, 0.0f, 0.0f)));
        }
        vertexList.addAll(zeroVertices);
        
        List<Face> faces = new ArrayList<Face>();
        for (int i = 0; i < vertexIndices.length; i += 4) {
            int faceVertexIndices[] = {vertexIndices[i] + nextVertexIndex, vertexIndices[i + 1] + nextVertexIndex, vertexIndices[i + 2] + nextVertexIndex, vertexIndices[i + 3] + nextVertexIndex};
            Face face = new Face(faceVertexIndices, new int[]{0, 0, 0, 0}, 1);
            faces.add(face);
        }

        return new Triplet<>(faces, vertexList, nextVertexIndex + zeroVertices.size());
    }

    private static Triplet<List<Face>, List<vec3>, Integer> modelOne(int bit, List<vec3> vertexList, int nextVertexIndex) {
        List<vec3> oneVertices = new ArrayList<vec3>() {
            {
                add(new vec3(0.070f, 0.000f, 0.000f)); // 0
                add(new vec3(0.070f, 0.000f, 0.024f)); // 1
                add(new vec3(0.040f, 0.000f, 0.039f)); // 2
                add(new vec3(0.040f, 0.000f, 0.017f)); // 3
                add(new vec3(0.093f, 0.000f, 0.000f)); // 4
                add(new vec3(0.093f, 0.000f, 0.185f)); // 5
                add(new vec3(0.070f, 0.000f, 0.185f)); // 6
            }
        };

        int vertexIndices[] = {
            0, 1, 2, 3,
            0, 4, 5, 6
        };

        for (int i = 0; i < oneVertices.size(); i++) {
            vec3 vertex = oneVertices.get(i);
            oneVertices.set(i, vertex.add(new vec3(bit * 0.118f, 0.0f, 0.0f)));
        }
        vertexList.addAll(oneVertices);
        
        List<Face> faces = new ArrayList<Face>();
        for (int i = 0; i < vertexIndices.length; i += 4) {
            int faceVertexIndices[] = {vertexIndices[i] + nextVertexIndex, vertexIndices[i + 1] + nextVertexIndex, vertexIndices[i + 2] + nextVertexIndex, vertexIndices[i + 3] + nextVertexIndex};
            Face face = new Face(faceVertexIndices, new int[]{0, 0, 0, 0}, 1);
            faces.add(face);
        }

        return new Triplet<>(faces, vertexList, nextVertexIndex + oneVertices.size());
    }
}
