public class CubeObjectGroup extends ObjectGroup {
    public Face front, back, right, left, top, bottom;

    public CubeObjectGroup(String id, Face front, Face back, Face right, Face left, Face top, Face bottom, String material) {
        super(id, material);
        this.front = front;
        this.back = back;
        this.right = right;
        this.left = left;
        this.top = top;
        this.bottom = bottom;
    }

    @Override
    public String objectString() {
        String objectString = "g " + id + "\n";
        objectString += "usemtl " + material + "\n";

        if (front.isPlane()) {
            objectString += front.faceString() + "\n";
        }
        if (back.isPlane()) {
            objectString += back.faceString() + "\n";
        }
        if (right.isPlane()) {
            objectString += right.faceString() + "\n";
        }
        if (left.isPlane()) {
            objectString += left.faceString() + "\n";
        }
        objectString += top.faceString() + "\n";
        if (!bottom.equals(top)) {
            objectString += bottom.faceString() + "\n";
        }
        return objectString;
    }
}