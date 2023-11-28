import static org.lwjgl.opengl.GL30.*;

public class VAO {
    public int id;

    public VAO() {
        id = glGenVertexArrays();
    }

    public void linkAttribute(VBO vbo, int layout, int numComponents, int type, int stride, long offset) {
        vbo.bind();
        glVertexAttribPointer(layout, numComponents, type, false, stride, offset);
        glEnableVertexAttribArray(layout);
        vbo.unbind();
    }

    public void bind() {
        glBindVertexArray(id);
    }

    public void unbind() {
        glBindVertexArray(0);
    }

    public void delete() {
        glDeleteVertexArrays(id);
    }
}
