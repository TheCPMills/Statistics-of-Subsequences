import java.util.*;

public class ObjectGroup {
    public String id;
    public List<Face> faces;
    public String material;
    
    public ObjectGroup(String id, String material) {
        this.id = id;
        this.faces = new ArrayList<>();
        this.material = material;
    }

    public ObjectGroup(String id, List<Face> faces, String material) {
        this.id = id;
        this.faces = faces;
        this.material = material;
    }

    public void addFaces(List<Face> faces) {
        this.faces.addAll(faces);
    }

    public String objectString() {
        if (faces.size() > 0) {
            String objectString = "g " + id + "\n";
            objectString += "usemtl " + material + "\n";

            for(Face face : faces) {
                objectString += face.faceString() + "\n";
            }
            return objectString;
        }
        return "";
    }
}
