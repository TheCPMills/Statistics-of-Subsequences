package glm;
import java.nio.FloatBuffer;

import org.lwjgl.BufferUtils;

public class vec2 {
    public float x;
    public float y;

    public vec2() {
        this.x = 0f;
        this.y = 0f;
    }

    public vec2(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public float lengthSquared() {
        return x * x + y * y;
    }

    public float length() {
        return (float) Math.sqrt(lengthSquared());
    }

    public vec2 normalize() {
        float length = length();
        return div(length);
    }

    public vec2 add(vec2 other) {
        float x = this.x + other.x;
        float y = this.y + other.y;
        return new vec2(x, y);
    }

    public vec2 negate() {
        return scale(-1f);
    }

    public vec2 sub(vec2 other) {
        return this.add(other.negate());
    }

    public vec2 scale(float scalar) {
        float x = this.x * scalar;
        float y = this.y * scalar;
        return new vec2(x, y);
    }

    public vec2 div(float scalar) {
        return scale(1f / scalar);
    }

    public float dot(vec2 other) {
        return this.x * other.x + this.y * other.y;
    }

    public FloatBuffer toBuffer() {
        FloatBuffer buffer = BufferUtils.createFloatBuffer(2);

        buffer.put(x);
        buffer.put(y);

        buffer.flip();
        
        return buffer;
    }
    
    public String toString() {
        return String.format("(%.4f, %.4f)", x, y);
    }

    public boolean equals(vec2 other) {
        return this.x == other.x && this.y == other.y;
    }
}