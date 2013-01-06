#ifndef RTVIEW_IMAGE_PPM_HXX
#define RTVIEW_IMAGE_PPM_HXX

#include "RTTL/common/RTInclude.hxx"
#include "RTTL/common/RTVec.hxx"
#include <vector>
#include <fstream>
#include <string>


class ImagePPM
{
public:

    ImagePPM(const char *fileName)
    {
        FILE* input = fopen(fileName, "r");

        if (!input) FATAL("Error couldn't open " << fileName);

        char buf[128];
        char line_buf[1024];

        fscanf(input, "%s", buf);

        // skip comment line
        int c = fgetc(input);
        while (c == '#' || isspace(c)) {
            if (c == '#')
                fgets(line_buf, sizeof(line_buf), input);
            c = fgetc(input);
        }
        ungetc(c, input);

        fscanf(input, "%u %u %u", &m_width, &m_height, &m_maxColor);
        fgetc(input); // get return

        printf("Loading texture %s : %dx%d\n", fileName, m_width, m_height);
        if ( m_width == 0 || m_height == 0 ) FATAL("Texture had zero width or height");
        
        m_data = aligned_malloc<unsigned char>(m_width * m_height * 3);

        if (!strcmp(buf, "P3"))
        {
            int r,g,b;

            for (int y = m_height-1; y >= 0; y--)
            {
                for (int x = 0; x < m_width; x++ )
                {
                    fscanf(input, "%i %i %i", &r, &g, &b);
                    int index = y * m_width + x;
                    m_data[3 * index + 0] = r;
                    m_data[3 * index + 1] = g;
                    m_data[3 * index + 2] = b;
                }
            }
        }
        else
        {
            unsigned char r,g,b;
            for (int y = m_height-1; y >= 0; y-- )
            {
                for (int x = 0; x < m_width; x++)
                {
                    r = fgetc(input);
                    g = fgetc(input);
                    b = fgetc(input);
                    int index = y * m_width + x;
                    m_data[3 * index + 0] = r;
                    m_data[3 * index + 1] = g;
                    m_data[3 * index + 2] = b;
                }
            }
        }

        fclose(input);
    }

  ~ImagePPM()
  {
    if (m_data) aligned_free(m_data);
  }

    int width() { return m_width; }
    int height() { return m_height; }
    unsigned char *data() { return m_data; }

private:
    int m_width;
    int m_height;
    int m_maxColor;
    unsigned char *m_data;
};
#endif

