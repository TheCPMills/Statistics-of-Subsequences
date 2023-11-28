package generation;
import java.util.*;

public class BinaryStringObjectGroup {
    String id;
    public List<Face> faces;
    public String material;
    
    public BinaryStringObjectGroup(String id, List<Face> faces, String material) {
        this.id = id;
        this.faces = faces;
        this.material = material;
    }

    public void addFaces(List<Face> faces) {
        this.faces.addAll(faces);
    }

    public String objectString() {
        String objectString = "g " + id + "\n";
        objectString += "usemtl " + material + "\n";

        for(Face face : faces) {
            objectString += face.faceString() + "\n";
        }
        return objectString;
    }
}
