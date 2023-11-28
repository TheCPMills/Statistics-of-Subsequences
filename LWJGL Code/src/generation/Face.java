package generation;

public class Face {
    public int[] vertexIndices;
    public int[] uvIndices;
    public int normalIndex;

    public Face(int[] vertexIndices) {
        this.vertexIndices = vertexIndices;
        this.uvIndices = null;
        this.normalIndex = -1;
    }

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

    public boolean isPlane() {
        int uniquePointsCount = 0;
        for(int i = 0; i < vertexIndices.length; i++) {
            boolean unique = true;
            for(int j = 0; j < vertexIndices.length; j++) {
                if(i != j && vertexIndices[i] == vertexIndices[j]) {
                    unique = false;
                }
            }
            if(unique) {
                uniquePointsCount++;
            }
        }
        return uniquePointsCount >= 3;
    }

    public boolean equals(Face other) {
        if(vertexIndices.length != other.vertexIndices.length) {
            return false;
        }
        for(int i = 0; i < vertexIndices.length; i++) {
            boolean contains = false;
            for(int j = 0; j < vertexIndices.length; j++) {
                if(vertexIndices[i] == other.vertexIndices[j]) {
                    contains = true;
                }
            }
            if(!contains) {
                return false;
            }
        }
        return true;
    }

    public String faceString() {
        String faceString = "f";
        for(int i = 0; i < vertexIndices.length; i++) {
            faceString += " " + (vertexIndices[i] + 1) + "/";
            if (uvIndices != null) {
                faceString += (uvIndices[i] + 1);
            }
            if (normalIndex != -1) {
                faceString += "/" + normalIndex;
            }
        }
        return faceString;
    }
}
