import glm.*;
import java.util.*;

public class Vertex {
    private vec3 position;
    private vec3 normal;
    private vec3 color;
    private vec2 uv;

    public Vertex(vec3 position, vec3 normal, vec3 color, vec2 uv) {
        this.position = position;
        this.normal = normal;
        this.color = color;
        this.uv = uv;
    }

    public vec3 position() {
        return position;
    }

    public vec3 normal() {
        return normal;
    }

    public vec3 color() {
        return color;
    }

    public vec2 uv() {
        return uv;
    }

    public static float[] vertexArray(List<Vertex> vertices) {
        float[] vertexArray = new float[vertices.size() * 11];
        for (int i = 0; i < vertices.size(); i++) {
            vertexArray[i * 11 + 0] = vertices.get(i).position.x;
            vertexArray[i * 11 + 1] = vertices.get(i).position.y;
            vertexArray[i * 11 + 2] = vertices.get(i).position.z;
            vertexArray[i * 11 + 3] = vertices.get(i).normal.x;
            vertexArray[i * 11 + 4] = vertices.get(i).normal.y;
            vertexArray[i * 11 + 5] = vertices.get(i).normal.z;
            vertexArray[i * 11 + 6] = vertices.get(i).color.x;
            vertexArray[i * 11 + 7] = vertices.get(i).color.y;
            vertexArray[i * 11 + 8] = vertices.get(i).color.z;
            vertexArray[i * 11 + 9] = vertices.get(i).uv.x;
            vertexArray[i * 11 + 10] = vertices.get(i).uv.y;
        }
        return vertexArray;
    }
}