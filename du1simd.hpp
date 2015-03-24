#ifndef BEDAS_SIMD_HPP
#define BEDAS_SIMD_HPP

#include <iterator>
#include <memory> 		//for align()

template <typename T, typename S>
class simd_vector_iterator;

template <typename T, typename S>
simd_vector_iterator<T, S> operator+ (simd_vector_iterator<T, S> it, std::ptrdiff_t n);

template <typename T, typename S>
simd_vector_iterator<T, S> operator+ (std::ptrdiff_t n, simd_vector_iterator<T, S> it);

template <typename T, typename S>
simd_vector_iterator<T, S> operator- (simd_vector_iterator<T, S> it, std::ptrdiff_t n);

template <typename T, typename S>
std::ptrdiff_t operator- (simd_vector_iterator<T, S> first, simd_vector_iterator<T, S> second);

template< typename T, typename S>
class simd_vector_iterator : public std::iterator<std::random_access_iterator_tag, T> 		
{
public:
	typedef simd_vector_iterator<T, S> sit;

	simd_vector_iterator() : position(NULL) {}
	simd_vector_iterator(T*  p_position) : position(p_position) {}

	T& operator* () {return *position;}
	T* operator-> () {return position;}
	T& operator[] (std::ptrdiff_t n) {return position[n];}
	
	bool operator==(sit other) {return (position == other.position);}
	bool operator!=(sit other) {return (position != other.position);}
	bool operator<(sit other) {return (position < other.position);}		//should be called only on iterators in same container
	bool operator>(sit other) {return (position > other.position);}
	bool operator<=(sit other) {return (position <= other.position);}
	bool operator>=(sit other) {return (position >= other.position);}
	
	friend sit operator+ <>(sit it, std::ptrdiff_t n);	
	friend sit operator+ <>(std::ptrdiff_t n, sit it);
	friend sit operator- <>(sit it, std::ptrdiff_t n);
	friend std::ptrdiff_t operator- <>(sit first, sit second);

	sit& operator++ () {position++; return *this;}
	sit operator++ (int) {sit result = *this; position++; return result;}	//postfix
	sit& operator-- () {position--; return *this;}
	sit operator-- (int) {sit result = *this; position--; return result;}

	sit& operator+= (std::ptrdiff_t n) {position += n; return *this;}
	sit& operator-= (std::ptrdiff_t n) {position -= n; return *this;}

	S* lower_block()
	{
		return reinterpret_cast<S*>(position - lower_offset());
	}
	S* upper_block()
	{
		return reinterpret_cast<S*>(position - upper_offset());
	}
	std::ptrdiff_t lower_offset()
	{
		std::ptrdiff_t result = ((std::intptr_t) position) % (sizeof(S));
		return result / sizeof(T);
	}
	std::ptrdiff_t upper_offset()
	{
		std::intptr_t k = sizeof(S)/sizeof(T);
		std::intptr_t t_pointer = (std::intptr_t)(position) / sizeof(T);
		std::intptr_t result = (((t_pointer % k) - k) % k);
		
		return result;

	}
	
private:
	T* position;
};

template <typename T, typename S>
simd_vector_iterator<T, S> operator+(simd_vector_iterator<T, S> it, std::ptrdiff_t n)
{
	return it += n;
}

template <typename T, typename S>
simd_vector_iterator<T, S> operator+(std::ptrdiff_t n, simd_vector_iterator<T, S> it)
{
	return it += n;
}

template <typename T, typename S>
simd_vector_iterator<T, S> operator-(simd_vector_iterator<T, S> it, std::ptrdiff_t n)
{
	return it -= n;
}

template <typename T, typename S>
std::ptrdiff_t operator-(simd_vector_iterator<T, S> first, simd_vector_iterator<T, S> second)
{
	return first.position - second.position;
}

template< typename T, typename S>
class simd_vector {
public:
	typedef simd_vector_iterator< T, S> iterator;
	typedef S* simd_iterator;

	explicit simd_vector(std::size_t s) : t_size(s)
	{
		int mod = s % (sizeof(S)/sizeof(T));
				//what if sizeof(T) != k*sizeof(S)??
		
		std::size_t space = s + mod + sizeof(S)/sizeof(T);		//sizeof(...) is max that can be aligned. "mod" because I want whole last big block allocated.
		allocated_memory = operator new(space*sizeof(T));

		void* pointer = allocated_memory;
		
		data = static_cast<T*>(pointer);
	}
	
	simd_vector(simd_vector&& other) : allocated_memory(other.allocated_memory), data(other.data), t_size(other.t_size)
	{		//move constructor
		other.data = NULL;
		other.allocated_memory = NULL;
		other.t_size = 0;
	}

	simd_vector& operator= (simd_vector&& other)
	{
		if(other == *this)
			return *this;
		
		operator delete(allocated_memory);

		data = other.data;
		allocated_memory = other.allocated_memory;
		t_size = other.t_size;

		other.data = NULL;
		other.allocated_memory = NULL;
		other.t_size = 0;
		return *this;
	}
	
	~simd_vector()
	{	
		operator delete(allocated_memory);
	}

	iterator begin()
	{
		return iterator(data);
	}

	iterator end()
	{	
		return iterator(data + t_size);
	}

	std::size_t size()
	{
		return t_size;
	}


private:
	void* allocated_memory;
	T* data;		//points to allocated_memory
	std::size_t t_size;	//number of T's in container
};

#endif
