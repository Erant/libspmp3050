import java.awt.Font;
import java.awt.FontFormatException;
import java.awt.GraphicsEnvironment;
import java.io.File;
import java.io.IOException;

public class BitmapFonts
{
	
	private static Font font;
	private static boolean preview, italic, bold, listFonts;
	private static String outputFileName, fontFileName, fontFamilyName;
	private static int fontSize = 16;
	
	@SuppressWarnings("serial")
	private static class argumentParseException extends RuntimeException
	{
		public argumentParseException(String message)
		{
			super(message);
		}
	}
	
	private static void parseArguments(String args[]) throws argumentParseException
	{
		
		try {
			for (int i=0; i<args.length; i++)
			{
				
				// output file name
				if (args[i].equals("-o"))
				{
					outputFileName = args[++i];
					continue;
				}
				
				// font input file
				if (args[i].equals("-f"))
				{
					fontFileName = args[++i];
					continue;
				}
				
				// font name
				if (args[i].equals("-n"))
				{
					fontFamilyName = args[++i];
					continue;
				}
				
				// font size
				if (args[i].equals("-s"))
				{
					try {
						fontSize = Integer.parseInt(args[++i]);
					} catch (NumberFormatException e)
					{
						throw new argumentParseException("Invalid font size: '" + args[i] + "'");
					}
					continue;
				}
				
				// preview font
				if (args[i].equals("-p"))
				{
					preview = true;
					continue;
				}
				
				// list available fonts
				if (args[i].equals("-l"))
				{
					listFonts = true;
					continue;
				}
				
				// set bold
				if (args[i].equals("-b"))
				{
					bold = true;
					continue;
				}
				
				// set italic
				if (args[i].equals("-i"))
				{
					italic = true;
					continue;
				}
				
				// unrecognised argument
				throw new argumentParseException("Unrecognised argument: " + args[i]);
				
			}
		} catch (ArrayIndexOutOfBoundsException ex)
		{
			throw new argumentParseException("Missing argument");
		}
		
		// check if we have the neccesary info
		if (!listFonts&&fontFileName==null&&fontFamilyName==null)
			throw new argumentParseException("No font family name or file name given");
		
	}

	public static void printBanner()
	{
		System.out.println("Bitmap font extractor for the SPMP project v.1");
	}

	public static void printUsage()
	{
		System.out.println("Examples: ");
		System.out.println("\tjava -jar bitmapfonts.jar -n arial -s 24 -o arial24.bfnt");
		System.out.println("\tjava -jar bitmapfonts.jar -f comfortaa.ttf -i -s 32");
		System.out.println("\tjava -jar bitmapfonts.jar -n \"Nimbus Mono L\" -s 12 -p");
		System.out.println();
		System.out.println("Switches: ");
		System.out.println("\t-b\t\t\tSet font weight to bold");
		System.out.println("\t-f [font file]\t\tUse a truetype (.ttf) file");
		System.out.println("\t-i\t\t\tEnable italics");
		System.out.println("\t-l\t\t\tList available fonts");
		System.out.println("\t-n [font family]\tSelect font by name (ie 'serif' or 'arial')");
		System.out.println("\t-o [file name]\t\tWrite output to [file name]");
		System.out.println("\t-p\t\t\tPreview the font (don't write output)");
		System.out.println("\t-s [size]\t\tSelect font size (defaults to 16pt)");
	}
	
	public static void listFonts()
	{
		System.out.println("----------------------------------------------------------------------");
		System.out.println(String.format("%-40s %s", "Family", "Name" ));
		System.out.println("----------------------------------------------------------------------");
		for (Font font : GraphicsEnvironment.getLocalGraphicsEnvironment().getAllFonts())
		{
			System.out.println(String.format("%-40s %s", font.getFamily(), font.getName() ));
		}
	}
	
	public static void preview()
	{
		FontPreview preview = new FontPreview(font);
		preview.setVisible(true);
	}
	
	public static void extractFont()
	{
		if (outputFileName==null)
		{
			if (fontFileName!=null)
				outputFileName = fontFileName.replaceAll(".ttf", "") + "_" + fontSize + ".bfnt";
			else
				outputFileName = fontFamilyName + "_" + fontSize + ".bfnt";
		}
		
		BitmapFontExtractor extractor = new BitmapFontExtractor(font, fontSize);
		extractor.extract();
		
		try
		{
			extractor.writeToFile(new File(outputFileName));
		} catch (IOException e)
		{
			System.err.println("Error writing output: " + e.getMessage());
			System.exit(-1);
		}
	}
	
	public static void main(String args[])
	{
		
		// parse command line arguments
		try 
		{
			parseArguments(args);
		} catch (argumentParseException e)
		{
			System.err.println(e.getMessage());
			printUsage();
			System.exit(-1);
		}
		
		if (listFonts)
			listFonts();
		else
		{
			
			// create font
			if (fontFileName!=null)
			{
				try
				{
					font = Font.createFont(Font.TRUETYPE_FONT, new File(fontFileName));
				} catch (FontFormatException e)
				{
					System.err.println("Format error loading font: "+ e.getMessage());
				} catch (IOException e)
				{
					System.err.println("I/O error loading font: "+ e.getMessage());
				}
			} else
			{
				font = new Font(fontFamilyName, Font.PLAIN , 1);
			}

			// set size and flags
			font = font.deriveFont((float)fontSize);
			font = font.deriveFont((bold?Font.BOLD:0) | (italic?Font.ITALIC:0));
			
			if (preview)
				preview();
			else
				extractFont();
			
		}

		
	}

}
