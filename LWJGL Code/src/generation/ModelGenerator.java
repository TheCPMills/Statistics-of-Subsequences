package generation;
import java.io.*;
import java.util.*;
import org.javatuples.*;
import glm.*;

public class ModelGenerator {
    public static void main(String[] args) throws Exception {
        for (int n = 1; n <= 5; n++) {
            for (int m = 1; m <= 5; m++) {
                generate(n, m);
            }
        }
    }

    public static void generate(int n, int m) throws Exception {
        FileWriter objWriter = new FileWriter(new File(System.getProperty("user.dir") + "/res/models/model_" + n + "x" + m +  ".obj"));
        genObj(n, m, objWriter);
        objWriter.close();

        FileWriter mtlWriter = new FileWriter(new File(System.getProperty("user.dir") + "/res/models/model_" + n + "x" + m +  ".mtl"));
        genMtl(n, m, mtlWriter);
        mtlWriter.close();
    }

    private static void genObj(int n, int m, FileWriter objWriter) throws Exception {
        objWriter.write("mtllib model_" + n + "x" + m + ".mtl\n\n");
        int[][] lcsMatrix = constructMatrix(n, m);
        int base = 0;

        HashMap<String, Integer> vertexMap = new HashMap<>();
        int nextVertexIndex = 0;
        
        List<vec3> vertices = new ArrayList<>();
        List<vec2> uvs = new ArrayList<>() {
            {
                add(new vec2(0.0f, 0.0f));
            }
        };
        List<vec3> normals = new ArrayList<>() {
            {
                add(new vec3(1.0f, 0.0f, 0.0f));
                add(new vec3(-1.0f, 0.0f, 0.0f));
                add(new vec3(0.0f, 1.0f, 0.0f));
                add(new vec3(0.0f, -1.0f, 0.0f));
                add(new vec3(0.0f, 0.0f, 1.0f));
                add(new vec3(0.0f, 0.0f, -1.0f));
            }
        };
        List<ObjectGroup> objects = new ArrayList<>();

        for(int x = 1; x < lcsMatrix.length - 1; x++) {
            for (int z = 1; z < lcsMatrix[x].length - 1; z++) {
                int y = lcsMatrix[x][z];
                String id = "Object(" + x + "," + z + ")[length_" + y + "]";
                Face front, back, right, left, top, bottom;
                String material = "length_" + y;

                int frontHeight = lcsMatrix[x][z - 1];
                int backHeight = lcsMatrix[x][z + 1];
                int rightHeight = lcsMatrix[x + 1][z];
                int leftHeight = lcsMatrix[x - 1][z];

                if (y <= frontHeight) {
                    frontHeight = y;
                }
                if (y <= backHeight) {
                    backHeight = y;
                }
                if (y <= rightHeight) {
                    rightHeight = y;
                }
                if (y <= leftHeight) {
                    leftHeight = y;
                }

                // front face vertices
                vec3 frontVertices[] = new vec3[] {
                    new vec3(x - 1, base + y, z - 1),
                    new vec3(x - 1, base + frontHeight, z - 1),
                    new vec3(x, base + frontHeight, z - 1),
                    new vec3(x, base + y, z - 1)
                };

                // back face vertices
                vec3 backVertices[] = new vec3[] {
                    new vec3(x, base + y, z),
                    new vec3(x, base + backHeight, z),
                    new vec3(x - 1, base + backHeight, z),
                    new vec3(x - 1, base + y, z)
                };

                // right face vertices
                vec3 rightVertices[] = new vec3[] {
                    new vec3(x, base + y, z - 1),
                    new vec3(x, base + rightHeight, z - 1),
                    new vec3(x, base + rightHeight, z),
                    new vec3(x, base + y, z)
                };

                // left face vertices
                vec3 leftVertices[] = new vec3[] {
                    new vec3(x - 1, base + y, z),
                    new vec3(x - 1, base + leftHeight, z),
                    new vec3(x - 1, base + leftHeight, z - 1),
                    new vec3(x - 1, base + y, z - 1)
                };

                // top face vertices
                vec3 topVertices[] = new vec3[] {
                    new vec3(x - 1, base + y, z),
                    new vec3(x - 1, base + y, z - 1),
                    new vec3(x, base + y, z - 1),
                    new vec3(x, base + y, z)
                };

                // bottom face vertices
                vec3 bottomVertices[] = new vec3[] {
                    new vec3(x, base, z),
                    new vec3(x, base, z - 1),
                    new vec3(x - 1, base, z - 1),
                    new vec3(x - 1, base, z)
                };

                // front 
                Quartet<Face, List<vec3>, HashMap<String, Integer>, Integer> frontFace = constructFace(frontVertices, vertices, vertexMap, 5, nextVertexIndex);
                front = frontFace.getValue0();
                vertices = frontFace.getValue1();
                vertexMap = frontFace.getValue2();
                nextVertexIndex = frontFace.getValue3();

                // back
                Quartet<Face, List<vec3>, HashMap<String, Integer>, Integer> backFace = constructFace(backVertices, vertices, vertexMap, 6, nextVertexIndex);
                back = backFace.getValue0();
                vertices = backFace.getValue1();
                vertexMap = backFace.getValue2();
                nextVertexIndex = backFace.getValue3();

                // right
                Quartet<Face, List<vec3>, HashMap<String, Integer>, Integer> rightFace = constructFace(rightVertices, vertices, vertexMap, 1, nextVertexIndex);
                right = rightFace.getValue0();
                vertices = rightFace.getValue1();
                vertexMap = rightFace.getValue2();
                nextVertexIndex = rightFace.getValue3();

                // left
                Quartet<Face, List<vec3>, HashMap<String, Integer>, Integer> leftFace = constructFace(leftVertices, vertices, vertexMap, 2, nextVertexIndex);
                left = leftFace.getValue0();
                vertices = leftFace.getValue1();
                vertexMap = leftFace.getValue2();
                nextVertexIndex = leftFace.getValue3();

                // top
                Quartet<Face, List<vec3>, HashMap<String, Integer>, Integer> topFace = constructFace(topVertices, vertices, vertexMap, 3, nextVertexIndex);
                top = topFace.getValue0();
                vertices = topFace.getValue1();
                vertexMap = topFace.getValue2();
                nextVertexIndex = topFace.getValue3();

                // bottom
                Quartet<Face, List<vec3>, HashMap<String, Integer>, Integer> bottomFace = constructFace(bottomVertices, vertices, vertexMap, 4, nextVertexIndex);
                bottom = bottomFace.getValue0();
                vertices = bottomFace.getValue1();
                vertexMap = bottomFace.getValue2();
                nextVertexIndex = bottomFace.getValue3();

                // create object mesh
                ObjectGroup object = new ObjectGroup(id, front, back, right, left, top, bottom, material);
                objects.add(object);
            }
        }

        // write vertices
        for(vec3 vertex : vertices) {
            objWriter.write("v " + vertex.x + " " + vertex.y + " " + vertex.z + "\n");
        }
        objWriter.write("\n");

        // write uvs
        for(vec2 uv : uvs) {
            objWriter.write("vt " + uv.x + " " + uv.y + "\n");
        }
        objWriter.write("\n");

        // write normals
        for(vec3 normal : normals) {
            objWriter.write("vn " + normal.x + " " + normal.y + " " + normal.z + "\n");
        }
        objWriter.write("\n");

        // write objects
        for(ObjectGroup object : objects) {
            objWriter.write(object.objectString() + "\n");
        }
    }

    private static void genMtl(int n, int m, FileWriter mtlWriter) throws Exception {
        int numColors = Math.min(n, m) + 1;
        Triplet<Float, Float, Float>[] colors = Gradient.generate(new int[]{0xfde724, 0x79d151, 0x29788e, 0x404387, 0x440154}, numColors);
        for (int i = 0; i < numColors; i++) {
            Triplet<Float, Float, Float> color = colors[i];

            mtlWriter.write("newmtl length_" + i + "\n");
            mtlWriter.write("Ka " + color.getValue0() + " " + color.getValue1() + " " + color.getValue2() + "\n");
            mtlWriter.write("Kd " + color.getValue0() + " " + color.getValue1() + " " + color.getValue2() + "\n");
            mtlWriter.write("Ks " + color.getValue0() + " " + color.getValue1() + " " + color.getValue2() + "\n");
            mtlWriter.write("Ke 0.0 0.0 0.0\n");
            mtlWriter.write("Ns 10.0\n");
            mtlWriter.write("Ni 1.45\n");
            mtlWriter.write("d 1.0\n");
            mtlWriter.write("illum 2\n");

            mtlWriter.write("\n");
        }
    }

    private static Quartet<Face, List<vec3>, HashMap<String, Integer>, Integer> constructFace(vec3[] vertices, List<vec3> vertexList, HashMap<String, Integer> vertexMap, int normalIndex, int nextVertexIndex) {
        int[] vertexIndices = new int[4];
        int[] uvIndices = {0, 0, 0, 0};

        for(int i = 0; i < 4; i++) {
            String vertexString = vertices[i].toString();
            if(vertexMap.containsKey(vertexString)) {
                vertexIndices[i] = vertexMap.get(vertexString);
            } else {
                vertexList.add(vertices[i]);
                vertexMap.put(vertexString, nextVertexIndex);
                vertexIndices[i] = nextVertexIndex++;
            }
        }

        return new Quartet<>(new Face(vertexIndices, uvIndices, normalIndex), vertexList, vertexMap, nextVertexIndex); 
    }

    private static int[][] constructMatrix(int n, int m) {
        List<String> n_strings = genStringSet(n);
        List<String> m_strings = genStringSet(m);
        int n_size = (int) Math.pow(2, n);
        int m_size = (int) Math.pow(2, m);

        int[][] matrix = new int[n_size + 2][m_size + 2];

        for(int i = 0; i < n_size; i++) {
            for(int j = 0; j < m_size; j++) {
                matrix[i + 1][j + 1] = lcs_dynamic_programming(n_strings.get(i), m_strings.get(j));
            }
        }

        return matrix;
    } 

    // ===============================================================================

    private static int lcs_dynamic_programming(String s1, String s2) {
        int m = s1.length();
        int n = s2.length();
        int[][] dp = new int[m + 1][n + 1];

        for(int i = 0; i <= m; i++) {
            for(int j = 0; j <= n; j++) {
                if(i == 0 || j == 0) {
                    dp[i][j] = 0;
                } else if(s1.charAt(i - 1) == s2.charAt(j - 1)) {
                    dp[i][j] = 1+dp[i - 1][j - 1];
                } else {
                    dp[i][j] = Math.max(
                        dp[i - 1][j],
                        dp[i][j - 1]
                    );
                }
            }
        }

        return dp[m][n];
    }

    private static List<String> genStringSet(int length) {
        // Generate all binary strings of length n
        List<String> strings = new ArrayList<>();
        for(int i=0; i<Math.pow(2, length); i++) {
            String s = Integer.toBinaryString(i);
            while(s.length() < length) {
                s = "0" + s;
            }
            strings.add(s);
        }
        return strings;
    }
}
