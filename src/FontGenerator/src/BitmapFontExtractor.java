import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

/**
 * 
 * Extracts a bitmap font from a .tff font. 
 * 
 */
public class BitmapFontExtractor
{

	private final Font font;
	private final int fontSize;

	private BufferedImage image;
	private Graphics2D g2;
	private ArrayList<Glyph> glyphs;
	
	public BitmapFontExtractor(Font font, int fontSize)
	{
		this.font = font;
		this.fontSize = fontSize;
	}
	
	private class Glyph
	{
		
		public byte[] data;
		public char c;
		public int w, h;
		public int horizontalShift, verticalShift;
			
		
	}
	
	private Glyph extractCharacter(char c)
	{
		g2.clearRect(0,0,image.getWidth(),image.getHeight());
		
        g2.drawString("" + c, fontSize, fontSize);
        int pixels[] = image.getRGB(0, 0, image.getWidth(), image.getHeight(), null, 0, image.getWidth());
        
        // find horizontal bounds
        int x,y;
        int horizontalStart = -1, horizontalEnd = -1;
        for (x=0; x<image.getWidth(); x++)
            for (y=0; y<image.getHeight(); y++)
            	if ((pixels[y*image.getWidth()+x]&0xffffff)!=0)
            	{
            		if (horizontalStart==-1)
            			horizontalStart = x;
            		else
                		horizontalEnd = x;
            	}

        // find vertical bounds
        int verticalStart = -1, verticalEnd = -1;
        for (y=0; y<image.getHeight(); y++)
            for (x=0; x<image.getWidth(); x++)
            	if ((pixels[y*image.getWidth()+x]&0xffffff)!=0)
            	{
            		if (verticalStart==-1)
            			verticalStart = y;
            		else
            			verticalEnd = y;
            	}
        
        if (horizontalStart==-1||horizontalEnd==-1||verticalStart==-1||verticalEnd==-1)
        	return null;

	//if (horizontalStart==horizontalEnd||verticalStart==verticalEnd) return null;
        
        BufferedImage glyphImage = image.getSubimage(horizontalStart, verticalStart, horizontalEnd-horizontalStart+1, verticalEnd-verticalStart+1);
        int glyphPixels[] = glyphImage.getRGB(0, 0, glyphImage.getWidth(), glyphImage.getHeight(), null, 0, glyphImage.getWidth());
        
        Glyph ret = new Glyph();
        ret.data = new byte[glyphPixels.length];
        for (int i=0; i<glyphPixels.length; i++) ret.data[i] = (byte)(glyphPixels[i] & 0xff);
        
        ret.w = glyphImage.getWidth();
        ret.h = glyphImage.getHeight();
        ret.horizontalShift = horizontalStart - fontSize;
        ret.verticalShift = verticalEnd - fontSize;
        ret.c = c;
        
        return ret;        
	}
	
	public void writeToFile(File file) throws IOException
	{
		if (glyphs==null)
			throw new IllegalStateException("Font has not been extracted yet, call extract() first");
		
		DataOutputStream out = new DataOutputStream(new FileOutputStream(file));
		
		// write header
		out.write((byte)'B');
		out.write((byte)'F');
		out.write((byte)'N');
		out.write((byte)'T');
		out.writeShort(glyphs.size());
		out.writeShort(fontSize);
		int totalDataSize = 0; 
		for (Glyph glyph : glyphs)
			totalDataSize += glyph.data.length;
		out.writeInt(totalDataSize);
		
		// write index
		int offset = 0;
		for (Glyph glyph : glyphs)
		{
			out.writeByte((byte)glyph.c);
			out.writeByte((byte)glyph.w);
			out.writeByte((byte)glyph.h);
			out.writeByte((byte)glyph.horizontalShift);
			out.writeByte((byte)glyph.verticalShift);
			out.writeInt(offset);
			offset += glyph.data.length;
		}
		
		// write data
		for (Glyph glyph : glyphs)
		{
			out.write(glyph.data);
		}
		
	}
	
	public void extract()
	{
		// create image buffer
		image = new BufferedImage(fontSize*3, fontSize*3, BufferedImage.TYPE_INT_RGB);
        g2 = image.createGraphics();
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_OFF);
        g2.setFont(font);
        g2.setColor(Color.white);
        
        glyphs = new ArrayList<Glyph>();
        
        for (int i=0; i<128; i++)
        	if (font.canDisplay((char)i))
        	{
        		Glyph glyph = extractCharacter((char)i); 
        		if (glyph!=null)
        			glyphs.add(glyph);
        	}
        
	}

}
