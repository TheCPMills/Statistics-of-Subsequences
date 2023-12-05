package glm;
import java.nio.*;
import org.lwjgl.*;

public class vec4 {
    public float x;
    public float y;
    public float z;
    public float w;
    public static final vec4 ZERO = new vec4(0f, 0f, 0f, 0f);

    public vec4() {
        this.x = 0f;
        this.y = 0f;
        this.z = 0f;
        this.w = 0f;
    }

    public vec4(float x, float y, float z, float w) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }

    public vec4(vec3 xyz, float w) {
        this.x = xyz.x;
        this.y = xyz.y;
        this.z = xyz.z;
        this.w = w;
    }

    public vec4 add(vec4 other) {
        float x = this.x + other.x;
        float y = this.y + other.y;
        float z = this.z + other.z;
        float w = this.w + other.w;
        return new vec4(x, y, z, w);
    }

    public vec4 sub(vec4 other) {
        return this.add(other.negate());
    }

    public vec4 scale(float scalar) {
        float x = this.x * scalar;
        float y = this.y * scalar;
        float z = this.z * scalar;
        float w = this.w * scalar;
        return new vec4(x, y, z, w);
    }

    public vec4 negate() {
        return scale(-1f);
    }

    public boolean equals(vec4 other) {
        return this.x == other.x && this.y == other.y && this.z == other.z && this.w == other.w;
    }

    public float length() {
        return (float) Math.sqrt(dot(this));
    }

    public float angle(vec4 other) {
        return (float) Math.acos(dot(other) / (length() * other.length()));
    }

    public vec4 normalize() {
        float length = length();
        if (length == 0f) {
            return this;
        }
        return scale(1.0f / length);
    }

    public float dot(vec4 other) {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    public FloatBuffer toBuffer() {
        FloatBuffer buffer = BufferUtils.createFloatBuffer(4);

        buffer.put(x);
        buffer.put(y);
        buffer.put(z);
        buffer.put(w);

        buffer.flip();

        return buffer;
    }

    public String toString() {
        return String.format("(%.4f, %.4f, %.4f, %.4f)", x, y, z, w);
    }
}