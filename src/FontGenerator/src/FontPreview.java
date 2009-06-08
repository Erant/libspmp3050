import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;

import javax.swing.JFrame;
import javax.swing.JPanel;


@SuppressWarnings("serial")
public class FontPreview extends JFrame
{
	
	private final Font font;
	
	public FontPreview(Font font)
	{
		this.font = font;
		this.setSize(800, 200);
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		
		
		this.add(new JPanel()
			{
			@Override
			public void paint(Graphics g)
			{
				drawFont(g);
			}
			}
		);
	}
	
	public void drawFont(Graphics g)
	{
		int baseline = 40;
		
		Graphics2D g2 = (Graphics2D)g.create();
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_OFF);

		g2.setColor(Color.white);
		g2.fillRect(0, 0, this.getWidth(), this.getHeight());

		g2.setColor(Color.lightGray);
		g2.drawLine(0, baseline + font.getSize(), this.getWidth(), baseline + font.getSize());
		
		g2.setColor(Color.black);
		g2.setFont(font);
		
		String str = "The quick brown fox jumps over the lazy dog.";
		g2.drawString(str, font.getSize(), baseline + font.getSize());
		
	}
	
	

}
