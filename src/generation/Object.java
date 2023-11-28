package generation;

public class Object {
    String id;
    public Face front, back, right, left, top, bottom;
    public String material;

    public Object(String id, Face front, Face back, Face right, Face left, Face top, Face bottom, String material) {
        this.id = id;
        this.front = front;
        this.back = back;
        this.right = right;
        this.left = left;
        this.top = top;
        this.bottom = bottom;
        this.material = material;
    }

    public String objectString() {
        String objectString = "o " + id + "\n";
        objectString += "usemtl " + material + "\n";
        objectString += front.faceString() + "\n";
        objectString += back.faceString() + "\n";
        objectString += right.faceString() + "\n";
        objectString += left.faceString() + "\n";
        objectString += top.faceString() + "\n";
        objectString += bottom.faceString() + "\n";
        return objectString;
    }
}