package glm;
import java.nio.FloatBuffer;

import org.lwjgl.BufferUtils;

public class mat2 {
    private float m00, m01;
    private float m10, m11;

    public mat2() {
        m00 = 0f;
        m01 = 0f;
        m10 = 0f;
        m11 = 0f;
    }

    public mat2(vec2 column0, vec2 column1) {
        set(column0, column1);
    }

    public void set(vec2 column0, vec2 column1) {
        m00 = column0.x;
        m10 = column0.y;

        m01 = column1.x;
        m11 = column1.y;
    }
    
    public void set(int columnNumber, vec2 column) {
        switch (columnNumber) {
            case 0:
                m00 = column.x;
                m10 = column.y;
                break;
            case 1:
                m01 = column.x;
                m11 = column.y;
                break;
            default:
                throw new IndexOutOfBoundsException();
        }
    }

    public mat2 add(mat2 other) {
        mat2 result = new mat2();

        result.m00 = this.m00 + other.m00;
        result.m10 = this.m10 + other.m10;

        result.m01 = this.m01 + other.m01;
        result.m11 = this.m11 + other.m11;

        return result;
    }

    public mat2 negate() {
        return mult(-1f);
    }

    public mat2 sub(mat2 other) {
        return this.add(other.negate());
    }

    public mat2 mult(float scalar) {
        mat2 result = new mat2();

        result.m00 = this.m00 * scalar;
        result.m10 = this.m10 * scalar;

        result.m01 = this.m01 * scalar;
        result.m11 = this.m11 * scalar;

        return result;
    }

    public vec2 mult(vec2 vector) {
        float x = this.m00 * vector.x + this.m01 * vector.y;
        float y = this.m10 * vector.x + this.m11 * vector.y;
        return new vec2(x, y);
    }

    public mat2 mult(mat2 other) {
        mat2 result = new mat2();

        result.m00 = this.m00 * other.m00 + this.m01 * other.m10;
        result.m10 = this.m10 * other.m00 + this.m11 * other.m10;

        result.m01 = this.m00 * other.m01 + this.m01 * other.m11;
        result.m11 = this.m10 * other.m01 + this.m11 * other.m11;

        return result;
    }

    public mat2 transpose() {
        mat2 result = new mat2();

        result.m00 = this.m00;
        result.m10 = this.m01;

        result.m01 = this.m10;
        result.m11 = this.m11;

        return result;
    }

    public FloatBuffer toBuffer() {
        FloatBuffer buffer = BufferUtils.createFloatBuffer(4);

        buffer.put(m00);
        buffer.put(m10);

        buffer.put(m01);
        buffer.put(m11);

        buffer.flip();
        
        return buffer;
    }

    public static final mat2 identity() {
        mat2 identityMatrix = new mat2();

        identityMatrix.m00 = 1f;
        identityMatrix.m11 = 1f;

        return identityMatrix;
    }
}