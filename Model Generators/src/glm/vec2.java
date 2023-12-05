package glm;
import java.nio.*;
import org.lwjgl.*;

public class vec2 {
    public float x;
    public float y;
    public static final vec2 ZERO = new vec2(0f, 0f);

    public vec2() {
        this.x = 0f;
        this.y = 0f;
    }

    public vec2(float x, float y) {
        this.x = x;
        this.y = y;
    }    

    public vec2 add(vec2 other) {
        float x = this.x + other.x;
        float y = this.y + other.y;
        return new vec2(x, y);
    }

    public vec2 sub(vec2 other) {
        return this.add(other.negate());
    }

    public vec2 scale(float scalar) {
        float x = this.x * scalar;
        float y = this.y * scalar;
        return new vec2(x, y);
    }

    public vec2 negate() {
        return scale(-1f);
    }

    public boolean equals(vec2 other) {
        return this.x == other.x && this.y == other.y;
    }

    public float length() {
        return (float) Math.sqrt(dot(this));
    }

    public float angle(vec2 other) {
        return (float) Math.acos(dot(other) / (length() * other.length()));
    }

    public vec2 normalize() {
        float length = length();
        if (length == 0f) {
            return this;
        }
        return scale(1.0f / length);
    }

    public float dot(vec2 other) {
        return x * other.x + y * other.y;
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
}