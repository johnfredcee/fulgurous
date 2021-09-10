
#pragma once

// look at http://coliru.stacked-crooked.com/a/454dc9ba82274528
/**
 * Class used to build a binary buffer from a sequence of scalar tuples,
 * OpenGL retained-mode style.
 */
template <typename T>
class BufferBuilder	{
public:
	using element_type = typename T;
	using component_type = typename T::type;

    /* Actual buffer */
	std::vector< element_type >  mBuffer;

	/*  Number of scalars per element (eg 2 for 2f, 4 for 4ui, etc */
	static constexpr GLsizei mComponentCount = element_type::dim;
	/* datatype of elements in the buffer */
	static constexpr GLenum  mType = GL_enum<component_type>::value;


	BufferBuilder(std::initializer_list<element_type> list)
	{
        mBuffer.reserve(list.size());
		std::initializer_list<element_type>::iterator it = list.begin();
		while(it != list.end())
		{
			mBuffer.push_back(*it);
			++it;
		}
	}

    BufferBuilder(GLsizei elementCount = 0)
    {
        mBuffer.reserve(elementCount);
    }

	~BufferBuilder() = default;

	void add(const element_type& vec)
	{
		mBuffer.push_back(vec);
	}

	template <typename... Args>
	void emplace(Args&&... args)
	{
		mBuffer.emplace_back(std::forward<Args>(args)...);
	}

	void add(Point3 point)
	{
		GLfloat v3[3];
		storeXYZ(point, &v3);
		mBuffer.push_back(v3);
	}

	void add(Vector3& vec)
	{
		GLfloat v3[3];
		storeXYZ(vec, &v3);
		mBuffer.push_back(v3);
	}

	void add(Vector4& vec)
	{
		GLfloat v4[4];
		storeXYZW(vec, &v4);
		mBuffer.push_back(v4);
	}

    const void* BufferBuilder::getData()
    {
        return (const void*)&mBuffer[0];
    }

    GLsizei BufferBuilder::elementCount()
    {
        return mBuffer.size();
    }

    GLsizei BufferBuilder::componentCount()
    {
        return mBuffer.size() * mComponentCount;
    }

    GLsizei BufferBuilder::byteSize()
    {
        return mBuffer.size() * mComponentCount * sizeof(component_type);
    }

	/* return the type of the buffer (type and component count) */
	std::tuple<GLenum, GLsizei> getTypeInfo()
	{
		return std::tuple<GLenum, GLsizei>{ mType, mComponentCount };
	}

	std::shared_ptr< Buffer<T> > make_buffer(GLenum target, GLenum usage = GL_STATIC_DRAW)
	{
		return ::produce_buffer<T>(target, getData(), (GLsizei) mBuffer.size(), usage);
	}

	void update_buffer(std::shared_ptr< Buffer<element_type> > buffer)
	{
		buuffer->update(getData());
	}
	
};

