package glm;
import java.nio.*;
import org.lwjgl.*;

public class vec3 {
    public float x;
    public float y;
    public float z;
    public static final vec3 ZERO = new vec3(0f, 0f, 0f);

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

    public vec3 add(vec3 other) {
        float x = this.x + other.x;
        float y = this.y + other.y;
        float z = this.z + other.z;
        return new vec3(x, y, z);
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

    public vec3 negate() {
        return scale(-1f);
    }

    public boolean equals(vec3 other) {
        return this.x == other.x && this.y == other.y && this.z == other.z;
    }

    public float length() {
        return (float) Math.sqrt(dot(this));
    }

    public float angle(vec3 other) {
        return (float) Math.acos(dot(other) / (length() * other.length()));
    }

    public vec3 normalize() {
        float length = length();
        if (length == 0f) {
            return this;
        }
        return scale(1.0f / length);
    }

    public float dot(vec3 other) {
        return x * other.x + y * other.y + z * other.z;
    }

    public vec3 cross(vec3 other) {
        float x = this.y * other.z - this.z * other.y;
        float y = this.z * other.x - this.x * other.z;
        float z = this.x * other.y - this.y * other.x;
        return new vec3(x, y, z);
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
}