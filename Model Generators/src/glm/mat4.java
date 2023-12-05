package glm;
import java.nio.FloatBuffer;
import org.lwjgl.BufferUtils;

public class mat4 {
    private float m00, m01, m02, m03;
    private float m10, m11, m12, m13;
    private float m20, m21, m22, m23;
    private float m30, m31, m32, m33;

    public mat4() {
        m00 = 0f;
        m01 = 0f;
        m02 = 0f;
        m03 = 0f;
        m10 = 0f;
        m11 = 0f;
        m12 = 0f;
        m13 = 0f;
        m20 = 0f;
        m21 = 0f;
        m22 = 0f;
        m23 = 0f;
        m30 = 0f;
        m31 = 0f;
        m32 = 0f;
        m33 = 0f;
    }

    public mat4(vec4 column0, vec4 column1, vec4 column2, vec4 column3) {
        set(column0, column1, column2, column3);
    }

    public void set(vec4 column0, vec4 column1, vec4 column2, vec4 column3) {
        m00 = column0.x;
        m10 = column0.y;
        m20 = column0.z;
        m30 = column0.w;

        m01 = column1.x;
        m11 = column1.y;
        m21 = column1.z;
        m31 = column1.w;

        m02 = column2.x;
        m12 = column2.y;
        m22 = column2.z;
        m32 = column2.w;

        m03 = column3.x;
        m13 = column3.y;
        m23 = column3.z;
        m33 = column3.w;
    }

    public void set(int columnNumber, vec4 column) {
        switch (columnNumber) {
            case 0:
                m00 = column.x;
                m10 = column.y;
                m20 = column.z;
                m30 = column.w;
                break;
            case 1:
                m01 = column.x;
                m11 = column.y;
                m21 = column.z;
                m31 = column.w;
                break;
            case 2:
                m02 = column.x;
                m12 = column.y;
                m22 = column.z;
                m32 = column.w;
                break;
            case 3:
                m03 = column.x;
                m13 = column.y;
                m23 = column.z;
                m33 = column.w;
                break;
            default:
                throw new IndexOutOfBoundsException();
        }
    }

    public vec4 getColumn(int column) {
        switch (column) {
            case 0:
                return new vec4(m00, m10, m20, m30);
            case 1:
                return new vec4(m01, m11, m21, m31);
            case 2:
                return new vec4(m02, m12, m22, m32);
            case 3:
                return new vec4(m03, m13, m23, m33);
            default:
                throw new IndexOutOfBoundsException();
        }
    }

    public vec4 getRow(int row) {
        switch (row) {
            case 0:
                return new vec4(m00, m01, m02, m03);
            case 1:
                return new vec4(m10, m11, m12, m13);
            case 2:
                return new vec4(m20, m21, m22, m23);
            case 3:
                return new vec4(m30, m31, m32, m33);
            default:
                throw new IndexOutOfBoundsException();
        }
    }

    public float get(int row, int column) {
        switch (row) {
            case 0:
                switch (column) {
                    case 0:
                        return m00;
                    case 1:
                        return m01;
                    case 2:
                        return m02;
                    case 3:
                        return m03;
                    default:
                        throw new IndexOutOfBoundsException();
                }
            case 1:
                switch (column) {
                    case 0:
                        return m10;
                    case 1:
                        return m11;
                    case 2:
                        return m12;
                    case 3:
                        return m13;
                    default:
                        throw new IndexOutOfBoundsException();
                }
            case 2:
                switch (column) {
                    case 0:
                        return m20;
                    case 1:
                        return m21;
                    case 2:
                        return m22;
                    case 3:
                        return m23;
                    default:
                        throw new IndexOutOfBoundsException();
                }
            case 3:
                switch (column) {
                    case 0:
                        return m30;
                    case 1:
                        return m31;
                    case 2:
                        return m32;
                    case 3:
                        return m33;
                    default:
                        throw new IndexOutOfBoundsException();
                }
            default:
                throw new IndexOutOfBoundsException();
        }
    }

    public mat4 add(mat4 other) {
        mat4 result = new mat4();

        result.m00 = this.m00 + other.m00;
        result.m10 = this.m10 + other.m10;
        result.m20 = this.m20 + other.m20;
        result.m30 = this.m30 + other.m30;

        result.m01 = this.m01 + other.m01;
        result.m11 = this.m11 + other.m11;
        result.m21 = this.m21 + other.m21;
        result.m31 = this.m31 + other.m31;

        result.m02 = this.m02 + other.m02;
        result.m12 = this.m12 + other.m12;
        result.m22 = this.m22 + other.m22;
        result.m32 = this.m32 + other.m32;

        result.m03 = this.m03 + other.m03;
        result.m13 = this.m13 + other.m13;
        result.m23 = this.m23 + other.m23;
        result.m33 = this.m33 + other.m33;

        return result;
    }

    public mat4 sub(mat4 other) {
        return this.add(other.negate());
    }

    public vec4 mult(vec4 other) {
        float x = this.m00 * other.x + this.m01 * other.y + this.m02 * other.z + this.m03 * other.w;
        float y = this.m10 * other.x + this.m11 * other.y + this.m12 * other.z + this.m13 * other.w;
        float z = this.m20 * other.x + this.m21 * other.y + this.m22 * other.z + this.m23 * other.w;
        float w = this.m30 * other.x + this.m31 * other.y + this.m32 * other.z + this.m33 * other.w;
        return new vec4(x, y, z, w);
    }

    public mat4 mult(mat4 other) {
        mat4 result = new mat4();

        result.m00 = this.m00 * other.m00 + this.m01 * other.m10 + this.m02 * other.m20 + this.m03 * other.m30;
        result.m10 = this.m10 * other.m00 + this.m11 * other.m10 + this.m12 * other.m20 + this.m13 * other.m30;
        result.m20 = this.m20 * other.m00 + this.m21 * other.m10 + this.m22 * other.m20 + this.m23 * other.m30;
        result.m30 = this.m30 * other.m00 + this.m31 * other.m10 + this.m32 * other.m20 + this.m33 * other.m30;

        result.m01 = this.m00 * other.m01 + this.m01 * other.m11 + this.m02 * other.m21 + this.m03 * other.m31;
        result.m11 = this.m10 * other.m01 + this.m11 * other.m11 + this.m12 * other.m21 + this.m13 * other.m31;
        result.m21 = this.m20 * other.m01 + this.m21 * other.m11 + this.m22 * other.m21 + this.m23 * other.m31;
        result.m31 = this.m30 * other.m01 + this.m31 * other.m11 + this.m32 * other.m21 + this.m33 * other.m31;

        result.m02 = this.m00 * other.m02 + this.m01 * other.m12 + this.m02 * other.m22 + this.m03 * other.m32;
        result.m12 = this.m10 * other.m02 + this.m11 * other.m12 + this.m12 * other.m22 + this.m13 * other.m32;
        result.m22 = this.m20 * other.m02 + this.m21 * other.m12 + this.m22 * other.m22 + this.m23 * other.m32;
        result.m32 = this.m30 * other.m02 + this.m31 * other.m12 + this.m32 * other.m22 + this.m33 * other.m32;

        result.m03 = this.m00 * other.m03 + this.m01 * other.m13 + this.m02 * other.m23 + this.m03 * other.m33;
        result.m13 = this.m10 * other.m03 + this.m11 * other.m13 + this.m12 * other.m23 + this.m13 * other.m33;
        result.m23 = this.m20 * other.m03 + this.m21 * other.m13 + this.m22 * other.m23 + this.m23 * other.m33;
        result.m33 = this.m30 * other.m03 + this.m31 * other.m13 + this.m32 * other.m23 + this.m33 * other.m33;

        return result;
    }

    public mat4 scale(float scalar) {
        mat4 result = new mat4();

        result.m00 = this.m00 * scalar;
        result.m10 = this.m10 * scalar;
        result.m20 = this.m20 * scalar;
        result.m30 = this.m30 * scalar;

        result.m01 = this.m01 * scalar;
        result.m11 = this.m11 * scalar;
        result.m21 = this.m21 * scalar;
        result.m31 = this.m31 * scalar;

        result.m02 = this.m02 * scalar;
        result.m12 = this.m12 * scalar;
        result.m22 = this.m22 * scalar;
        result.m32 = this.m32 * scalar;

        result.m03 = this.m03 * scalar;
        result.m13 = this.m13 * scalar;
        result.m23 = this.m23 * scalar;
        result.m33 = this.m33 * scalar;

        return result;
    }

    public mat4 negate() {
        return scale(-1f);
    }

    public boolean equals(mat4 other) {
        return this.m00 == other.m00 && this.m10 == other.m10 && this.m20 == other.m20 && this.m30 == other.m30 &&
               this.m01 == other.m01 && this.m11 == other.m11 && this.m21 == other.m21 && this.m31 == other.m31 &&
               this.m02 == other.m02 && this.m12 == other.m12 && this.m22 == other.m22 && this.m32 == other.m32 &&
               this.m03 == other.m03 && this.m13 == other.m13 && this.m23 == other.m23 && this.m33 == other.m33;
    }

    public mat4 transpose() {
        mat4 result = new mat4();

        result.m00 = this.m00;
        result.m10 = this.m01;
        result.m20 = this.m02;
        result.m30 = this.m03;

        result.m01 = this.m10;
        result.m11 = this.m11;
        result.m21 = this.m12;
        result.m31 = this.m13;

        result.m02 = this.m20;
        result.m12 = this.m21;
        result.m22 = this.m22;
        result.m32 = this.m23;

        result.m03 = this.m30;
        result.m13 = this.m31;
        result.m23 = this.m32;
        result.m33 = this.m33;

        return result;
    }
    
    public float determinant() {
        float result = 0f;

        result += this.m00 * (this.m11 * (this.m22 * this.m33 - this.m23 * this.m32) - this.m12 * (this.m21 * this.m33 - this.m23 * this.m31) + this.m13 * (this.m21 * this.m32 - this.m22 * this.m31));
        result -= this.m01 * (this.m10 * (this.m22 * this.m33 - this.m23 * this.m32) - this.m12 * (this.m20 * this.m33 - this.m23 * this.m30) + this.m13 * (this.m20 * this.m32 - this.m22 * this.m30));
        result += this.m02 * (this.m10 * (this.m21 * this.m33 - this.m23 * this.m31) - this.m11 * (this.m20 * this.m33 - this.m23 * this.m30) + this.m13 * (this.m20 * this.m31 - this.m21 * this.m30));
        result -= this.m03 * (this.m10 * (this.m21 * this.m32 - this.m22 * this.m31) - this.m11 * (this.m20 * this.m32 - this.m22 * this.m30) + this.m12 * (this.m20 * this.m31 - this.m21 * this.m30));

        return result;
    }

    public mat4 inverse() {
        float determinant = determinant();
        if (determinant == 0f) {
            throw new ArithmeticException("Matrix is not invertible.");
        }

        mat4 result = new mat4();

        result.m00 = (this.m11 * (this.m22 * this.m33 - this.m23 * this.m32) - this.m12 * (this.m21 * this.m33 - this.m23 * this.m31) + this.m13 * (this.m21 * this.m32 - this.m22 * this.m31)) / determinant;
        result.m10 = (this.m10 * (this.m22 * this.m33 - this.m23 * this.m32) - this.m12 * (this.m20 * this.m33 - this.m23 * this.m30) + this.m13 * (this.m20 * this.m32 - this.m22 * this.m30)) / determinant;
        result.m20 = (this.m10 * (this.m21 * this.m33 - this.m23 * this.m31) - this.m11 * (this.m20 * this.m33 - this.m23 * this.m30) + this.m13 * (this.m20 * this.m31 - this.m21 * this.m30)) / determinant;
        result.m30 = (this.m10 * (this.m21 * this.m32 - this.m22 * this.m31) - this.m11 * (this.m20 * this.m32 - this.m22 * this.m30) + this.m12 * (this.m20 * this.m31 - this.m21 * this.m30)) / determinant;

        result.m01 = (this.m01 * (this.m22 * this.m33 - this.m23 * this.m32) - this.m02 * (this.m21 * this.m33 - this.m23 * this.m31) + this.m03 * (this.m21 * this.m32 - this.m22 * this.m31)) / determinant;
        result.m11 = (this.m00 * (this.m22 * this.m33 - this.m23 * this.m32) - this.m02 * (this.m20 * this.m33 - this.m23 * this.m30) + this.m03 * (this.m20 * this.m32 - this.m22 * this.m30)) / determinant;
        result.m21 = (this.m00 * (this.m21 * this.m33 - this.m23 * this.m31) - this.m01 * (this.m20 * this.m33 - this.m23 * this.m30) + this.m03 * (this.m20 * this.m31 - this.m21 * this.m30)) / determinant;
        result.m31 = (this.m00 * (this.m21 * this.m32 - this.m22 * this.m31) - this.m01 * (this.m20 * this.m32 - this.m22 * this.m30) + this.m02 * (this.m20 * this.m31 - this.m21 * this.m30)) / determinant;

        result.m02 = (this.m01 * (this.m12 * this.m33 - this.m13 * this.m32) - this.m02 * (this.m11 * this.m33 - this.m13 * this.m31) + this.m03 * (this.m11 * this.m32 - this.m12 * this.m31)) / determinant;
        result.m12 = (this.m00 * (this.m12 * this.m33 - this.m13 * this.m32) - this.m02 * (this.m10 * this.m33 - this.m13 * this.m30) + this.m03 * (this.m10 * this.m32 - this.m12 * this.m30)) / determinant;
        result.m22 = (this.m00 * (this.m11 * this.m33 - this.m13 * this.m31) - this.m01 * (this.m10 * this.m33 - this.m13 * this.m30) + this.m03 * (this.m10 * this.m31 - this.m11 * this.m30)) / determinant;
        result.m32 = (this.m00 * (this.m11 * this.m32 - this.m12 * this.m31) - this.m01 * (this.m10 * this.m32 - this.m12 * this.m30) + this.m02 * (this.m10 * this.m31 - this.m11 * this.m30)) / determinant;

        result.m03 = (this.m01 * (this.m12 * this.m23 - this.m13 * this.m22) - this.m02 * (this.m11 * this.m23 - this.m13 * this.m21) + this.m03 * (this.m11 * this.m22 - this.m12 * this.m21)) / determinant;
        result.m13 = (this.m00 * (this.m12 * this.m23 - this.m13 * this.m22) - this.m02 * (this.m10 * this.m23 - this.m13 * this.m20) + this.m03 * (this.m10 * this.m22 - this.m12 * this.m20)) / determinant;
        result.m23 = (this.m00 * (this.m11 * this.m23 - this.m13 * this.m21) - this.m01 * (this.m10 * this.m23 - this.m13 * this.m20) + this.m03 * (this.m10 * this.m21 - this.m11 * this.m20)) / determinant;
        result.m33 = (this.m00 * (this.m11 * this.m22 - this.m12 * this.m21) - this.m01 * (this.m10 * this.m22 - this.m12 * this.m20) + this.m02 * (this.m10 * this.m21 - this.m11 * this.m20)) / determinant;

        return result;
    }

    public FloatBuffer toBuffer() {
        FloatBuffer buffer = BufferUtils.createFloatBuffer(16);

        buffer.put(m00);
        buffer.put(m10);
        buffer.put(m20);
        buffer.put(m30);

        buffer.put(m01);
        buffer.put(m11);
        buffer.put(m21);
        buffer.put(m31);

        buffer.put(m02);
        buffer.put(m12);
        buffer.put(m22);
        buffer.put(m32);

        buffer.put(m03);
        buffer.put(m13);
        buffer.put(m23);
        buffer.put(m33);

        buffer.flip();

        return buffer;
    }

    public static final mat4 identity() {
        mat4 identityMatrix = new mat4();

        identityMatrix.m00 = 1f;
        identityMatrix.m11 = 1f;
        identityMatrix.m22 = 1f;
        identityMatrix.m33 = 1f;

        return identityMatrix;
    }
}