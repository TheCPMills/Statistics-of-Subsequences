package generation;

public class Face {
    public int[] vertexIndices;
    public int[] uvIndices;
    public int normalIndex;

    public Face(int[] vertexIndices, int normalIndex) {
        this.vertexIndices = vertexIndices;
        this.uvIndices = null;
        this.normalIndex = normalIndex;
    }

    public Face(int[] vertexIndices, int[] uvIndices, int normalIndex) {
        this.vertexIndices = vertexIndices;
        this.uvIndices = uvIndices;
        this.normalIndex = normalIndex;
    }

    public String faceString() {
        String faceString = "f";
        for(int i = 0; i < vertexIndices.length; i++) {
            faceString += " " + (vertexIndices[i] + 1) + "/";
            if(uvIndices != null) {
                faceString += (uvIndices[i] + 1);
            }
            faceString += "/" + normalIndex;

        }
        return faceString;
    }
}
