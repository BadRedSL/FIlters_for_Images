#pragma once
#include <QImage>
#include <math.h>

class Filter
{
protected:
	virtual QColor calcNewPixelColor(const QImage& img, int x, int y) const = 0;

public:
	virtual ~Filter() = default;
	virtual QImage process(const QImage& img) const;

};


//Mathematical morphology


class StructuralElement
{
public:

	std::unique_ptr<bool[]>data;
	int size;
	int line;
	int row;

	StructuralElement()
	{
		size = 9;
		line = 3;
		row = 3;

		data = std::make_unique<bool[]>(9);
		for (int i = 0; i < 9; i++)
			data[i] = 1;
	}

	template <size_t N>
	StructuralElement(bool (&origin) [N], int line)
	{
		size = N;
		this->line = line;
		row = size / line;
		data = std::make_unique<bool[]>(size);
		for (int i = 0; i < size; i++)
			data[i] = origin[i];
	}

};


class MatMorph
{
protected:
	virtual QColor calcNewPixelColor(const QImage& img, int x, int y, StructuralElement& Matrix) const = 0;
public:
	virtual ~MatMorph() = default;
	virtual QImage process(const QImage & img, StructuralElement& Matrix = StructuralElement());
};


class Erosion : public MatMorph
{
	 QColor calcNewPixelColor(const QImage& img, int x, int y, StructuralElement& Matrix) const override;
};


class Dilation : public MatMorph
{
	QColor calcNewPixelColor(const QImage& img, int x, int y, StructuralElement& Matrix) const override;
};


class Opening : public Erosion
{
public:
	QImage process(const QImage& img, StructuralElement& Matrix = StructuralElement()) override;
};


class Closing : public Dilation
{
public:
	QImage process(const QImage& img, StructuralElement& Matrix = StructuralElement()) override;
};


class Grad : public MatMorph
{
	QColor calcNewPixelColor(const QImage& img, int x, int y, StructuralElement& Matrix) const override;
};


//Point filters


class InvertFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;
		
};


class GrayScaleFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;

};


class SepiaFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;

};


class BrightFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;
};


class PerfectReflectorFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;
};


class WaveFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;
};


class CarryoverFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;
};


class StretchingHistogramFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;
};


class MedianFilter : public Filter
{
	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;
};


//Matrix filters


class Kernel
{
protected:

	std::unique_ptr<float[]>data;
	std::size_t radius;
	std::size_t getLen() const { return getSize() * getSize(); }

public:

	Kernel(std::size_t radius) : radius(radius)
	{
		data = std::make_unique<float[]>(getLen());
	}

	Kernel(const Kernel& other) : Kernel(other.radius)
	{
		std::copy(other.data.get(), other.data.get() + getLen(), data.get());
	}

	std::size_t getRadius() const { return radius; }
	std::size_t getSize() const { return 2 * radius + 1; }
	float operator[](std::size_t id) const { return data[id]; }
	float& operator [] (std::size_t id) { return data[id]; }
};


class MatrixFilter : public Filter
{
protected:
	Kernel mKernel;
	Kernel mKernel2=mKernel;
	QColor calcNewPixelColor(const QImage& img, int x, int y)const override;

public:
	MatrixFilter(const Kernel& kernel) : mKernel(kernel) {};
	MatrixFilter(const Kernel& kernel1, const Kernel&  kernel2) : mKernel(kernel1), mKernel2 (kernel2) {};
	virtual ~MatrixFilter() = default;

};


class BlurKernel : public Kernel
{
public:
	using Kernel::Kernel;
	BlurKernel(std::size_t radius = 2) :Kernel(radius)
	{
		for (std::size_t i = 0; i < getLen(); i++)
			data[i] = 1.0f / getLen();
	}
};

class BlurFilter : public MatrixFilter
{
public:
	BlurFilter(std::size_t radius =1): MatrixFilter(BlurKernel(radius)) {}
};


class GaussianKernel : public Kernel
{
public:
	using Kernel::Kernel;
	GaussianKernel(std::size_t radius = 2, float sigma = 3.f) :Kernel(radius)
	{
		float norm = 0;
		int signed_radius = static_cast<int>(radius);
		for (int x = -signed_radius; x <= signed_radius; ++x)
			for (int y = -signed_radius; y <= signed_radius; ++y)
			{
				std::size_t idx = (x + radius) * getSize() + (y + radius);
				data[idx] = std::exp(-(x * x + y * y) / (2 * sigma * sigma));
				norm += data[idx];
			}

		for (std::size_t i = 0; i < getLen(); ++i)
		{
			data[i] /= norm;
		}

	}
};

class GaussianFilter : public MatrixFilter
{
public:
	GaussianFilter(std::size_t radius = 1) :MatrixFilter(GaussianKernel(radius)) {}
}; 


class SharpnessKernel : public Kernel
{
public:
	using Kernel::Kernel;
	SharpnessKernel(std::size_t radius = 2) :Kernel(radius)
	{
		data[0] = 0; data[1] = -1; data[2] = 0;
		data[3] = -1; data[4] = 5; data[5] = -1;
		data[6] = 0; data[7] = -1; data[8] = 0;
	}
};

class SharpnessFilter : public MatrixFilter
{
	public:
		SharpnessFilter(std::size_t radius = 1) : MatrixFilter(SharpnessKernel(radius)) {}
};


class NewSharpnessKernel : public Kernel
{
public:
	using Kernel::Kernel;
	NewSharpnessKernel(std::size_t radius = 2) :Kernel(radius)
	{
		data[0] = -1; data[1] = -1; data[2] = -1;
		data[3] = -1; data[4] =  9; data[5] = -1;
		data[6] = -1; data[7] = -1; data[8] = -1;
	}
};

class NewSharpnessFilter: public MatrixFilter
{
public:
	NewSharpnessFilter(std::size_t radius = 1) : MatrixFilter(NewSharpnessKernel(radius)) {}
};


class EmbossingKernel : public Kernel
{
public:
	using Kernel::Kernel;
	EmbossingKernel(std::size_t radius = 2) :Kernel(radius)
	{
		data[0] = 0; data[1] = 1; data[2] = 0;
		data[3] = 1; data[4] = 0; data[5] = -1;
		data[6] = 0; data[7] = -1; data[8] = 0;
	}
};

class EmbossingFilter: public MatrixFilter
{
public:
	EmbossingFilter(std::size_t radius = 1) : MatrixFilter(EmbossingKernel(radius)) {}
	QColor calcNewPixelColor(const QImage& img, int x, int y)const override;
};


class SobelXKernel : public Kernel
{
public:
	using Kernel::Kernel;
	SobelXKernel(std::size_t radius = 1) :Kernel(radius)
	{
		data[0] = -1; data[1] = -2; data[2] = -1;
		data[3] = 0; data[4] = 0; data[5] = 0;
		data[6] = 1; data[7] = 2; data[8] = 1;
	}
};

class SobelXFilter : public MatrixFilter
{
public:
	SobelXFilter(std::size_t radius = 1) : MatrixFilter(SobelXKernel(radius)) {}
};


class SobelYKernel: public Kernel
{
public:
	using Kernel::Kernel;
	SobelYKernel(std::size_t radius = 1) :Kernel(radius)
	{
		data[0] = -1; data[1] = 0; data[2] = 1;
		data[3] = -2; data[4] = 0; data[5] = 2;
		data[6] = -1; data[7] = 0; data[8] = 1;
	}
};

class SobelYFilter : public MatrixFilter
{
public:
	SobelYFilter(std::size_t radius = 1) : MatrixFilter(SobelYKernel(radius)) {}
};


class SobelFilter : public MatrixFilter
{
public:

	QColor calcNewPixelColor(const QImage& img, int x, int y) const override;
	SobelFilter(std::size_t radius = 1) : MatrixFilter(SobelXKernel(radius), SobelYKernel(radius)) {}	
};


