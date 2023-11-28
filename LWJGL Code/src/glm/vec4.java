package glm;
import java.nio.FloatBuffer;

import org.lwjgl.BufferUtils;

public class vec4 {
    public float x;
    public float y;
    public float z;
    public float w;

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

    public float lengthSquared() {
        return x * x + y * y + z * z + w * w;
    }

    public float length() {
        return (float) Math.sqrt(lengthSquared());
    }

    public vec4 normalize() {
        float length = length();
        return div(length);
    }

    public vec4 add(vec4 other) {
        float x = this.x + other.x;
        float y = this.y + other.y;
        float z = this.z + other.z;
        float w = this.w + other.w;
        return new vec4(x, y, z, w);
    }

    public vec4 negate() {
        return scale(-1f);
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

    public vec4 div(float scalar) {
        return scale(1f / scalar);
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