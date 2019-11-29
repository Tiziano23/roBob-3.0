namespace math
{
class Vector
{
private:
    int size;
    double *values;

public:
    Vector() {}
    Vector(int size) : size(size)
    {
        values = new double[size];
        for (int i = 0; i < size; i++)
        {
            values[i] = 0;
        }
    }
    Vector(int size, double values_[]) : size(size)
    {
        values = new double[size];
        for (int i = 0; i < size; i++)
        {
            values[i] = values_[i];
        }
    }

    void add(double scalar)
    {
        for (int i = 0; i < size; i++)
        {
            values[i] += scalar;
        }
    }
    void add(Vector *v)
    {
        if (v->size != size)
            return;
        for (int i = 0; i < size; i++)
        {
            values[i] += v->values[i];
        }
    }
    void multiply(double scalar)
    {
        for (int i = 0; i < size; i++)
        {
            values[i] *= scalar;
        }
    }
    void multiply(Vector *v)
    {
        if (v->size != size)
            return;
        for (int i = 0; i < size; i++)
        {
            values[i] *= v->values[i];
        }
    }
    void map(double (*func)(double))
    {
        for (int i = 0; i < size; i++)
        {
            values[i] = func(values[i]);
        }
    }

    inline int getSize() { return size; }
    inline double getValue(int index) { return values[index]; }
    inline void setValue(int index, double value) { values[index] = value; }
    inline double *getValues() { return values; }

    static Vector map(Vector *v, double (*func)(double))
    {
        Vector result(v->size);
        for (int i = 0; i < result.size; i++)
        {
            result.setValue(i, func(v->getValue(i)));
        }
        return result;
    }
    static Vector add(Vector *v, Vector *w)
    {
        if (v->size != w->size)
            return Vector();
        Vector result(v->size);
        for (int i = 0; i < v->size; i++)
        {
            result.setValue(i, v->getValue(i) + w->getValue(i));
        }
        return result;
    }
    static Vector subtract(Vector *v, Vector *w)
    {
        if (v->size != w->size)
            return Vector();
        Vector result(v->size);
        for (int i = 0; i < v->size; i++)
        {
            result.setValue(i, v->getValue(i) - w->getValue(i));
        }
        return result;
    }
    static double dot(Vector *v, Vector *w)
    {
        if (v->size != w->size)
            return 0;
        double result = 0;
        for (int i = 0; i < v->size; i++)
        {
            result += v->getValue(i) * w->getValue(i);
        }
        return result;
    }
};
class Matrix
{
private:
    int size;
    int rows;
    int columns;
    double *values;

public:
    Matrix() {}
    Matrix(int columns, int rows) : columns(columns), rows(rows)
    {
        size = rows * columns;
        values = new double[size];
        for (int i = 0; i < size; i++)
        {
            values[i] = 0;
        }
    }
    Matrix(int columns, int rows, double values_[]) : columns(columns), rows(rows)
    {
        size = rows * columns;
        values = new double[size];
        for (int i = 0; i < size; i++)
        {
            values[i] = values_[i];
        }
    }

    void add(double scalar)
    {
        for (int i = 0; i < size; i++)
        {
            values[i] += scalar;
        }
    }
    void add(Matrix *m)
    {
        if (rows != m->rows || columns != m->columns)
            return;
        for (int i = 0; i < size; i++)
        {
            values[i] += m->values[i];
        }
    }
    void multiply(double scalar)
    {
        for (int i = 0; i < size; i++)
        {
            values[i] *= scalar;
        }
    }

    inline double getValue(int row, int column) { return values[row * columns + column]; }
    inline void setValue(int row, int column, double value) { values[row * columns + column] = value; }
    inline double *getValues() { return values; }
    inline int getRows() { return rows; }
    inline int getColumns() { return columns; }

    static Matrix rowMatrixFromVector(Vector *v)
    {
        return Matrix(v->getSize(), 1, v->getValues());
    }
    static Matrix columnMatrixFromVector(Vector *v)
    {
        return Matrix(1, v->getSize(), v->getValues());
    }

    static Matrix map(Matrix *m, double (*func)(double))
    {
        Matrix result(m->columns, m->rows);
        for (int i = 0; i < result.size; i++)
        {
            result.values[i] = func(m->values[i]);
        }
        return result;
    }
    static Matrix multiply(Matrix *a, Matrix *b)
    {
        if (a->columns != b->rows)
            return Matrix();
        Matrix result(b->columns, a->rows);
        for (int i = 0; i < result.rows; i++)
        {
            for (int j = 0; j < result.columns; j++)
            {
                Vector v(a->columns);
                Vector w(b->rows);
                for (int k = 0; k < a->columns; k++)
                {
                    v.setValue(k, a->getValue(i, k));
                    w.setValue(k, b->getValue(k, j));
                }
                result.setValue(i, j, math::Vector::dot(&v, &w));
            }
        }
        return result;
    }
    static Vector multiply(Matrix *m, Vector *v)
    {
        if (m->columns != v->getSize())
            return Vector();
        Vector result(m->rows);
        for (int i = 0; i < m->rows; i++)
        {
            double *w_values = new double[m->columns];
            for (int j = 0; j < m->columns; j++)
            {
                w_values[j] = m->getValue(i, j);
            }
            Vector w(m->columns, w_values);
            result.setValue(i, Vector::dot(v, &w));
            delete w_values;
        }
        return result;
    }
};
} // namespace math