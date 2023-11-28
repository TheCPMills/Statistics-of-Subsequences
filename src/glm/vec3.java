package glm;
import java.nio.FloatBuffer;

import org.lwjgl.BufferUtils;

public class vec3 {
    public float x;
    public float y;
    public float z;

    public vec3() {
        this.x = 0f;
        this.y = 0f;
        this.z = 0f;
    }

    public vec3(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public float lengthSquared() {
        return x * x + y * y + z * z;
    }

    public float length() {
        return (float) Math.sqrt(lengthSquared());
    }

    public vec3 normalize() {
        float length = length();
        if (length == 0f) {
            return this;
        }
        return div(length);
    }

    public vec3 add(vec3 other) {
        float x = this.x + other.x;
        float y = this.y + other.y;
        float z = this.z + other.z;
        return new vec3(x, y, z);
    }

    public vec3 negate() {
        return scale(-1f);
    }

    public vec3 sub(vec3 other) {
        return this.add(other.negate());
    }

    public vec3 scale(float scalar) {
        float x = this.x * scalar;
        float y = this.y * scalar;
        float z = this.z * scalar;
        return new vec3(x, y, z);
    }

    public vec3 div(float scalar) {
        return scale(1f / scalar);
    }

    public FloatBuffer toBuffer() {
        FloatBuffer buffer = BufferUtils.createFloatBuffer(3);

        buffer.put(x);
        buffer.put(y);
        buffer.put(z);

        buffer.flip();
        
        return buffer;
    }

    public String toString() {
        return String.format("(%.4f, %.4f, %.4f)", x, y, z);
    }

    public boolean equals(vec3 other) {
        return this.x == other.x && this.y == other.y && this.z == other.z;
    }
}