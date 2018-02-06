//GD-zoomover 
//bugs -capture zoomed object. zoom-while play..speed to be improved,
//zooming while recording - crashes the savefile
//pending:  timebar-for play, resizing playbackvideo, capture - filelocations+systemtime
//laptop cam : 0, usb webcam:1

package GyanDataBoroscope;

import java.awt.BorderLayout;
import java.awt.Graphics;

import com.sun.media.ui.MessageBox;
import com.xuggle.*;
import com.xuggle.mediatool.IMediaWriter;
import com.xuggle.mediatool.IMediaReader;
import com.xuggle.mediatool.IMediaViewer;
import com.xuggle.mediatool.MediaListenerAdapter;
import com.xuggle.mediatool.ToolFactory;
import com.xuggle.mediatool.event.IVideoPictureEvent;
import com.xuggle.xuggler.ICodec;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Image;
import java.awt.LayoutManager;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;

import javax.imageio.ImageIO;
import javax.swing.*;

import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.highgui.Highgui;
import org.opencv.highgui.VideoCapture;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.awt.event.MouseMotionAdapter;
import java.awt.event.MouseEvent;

public class Gui_Complete {
	private static final String BUTTON_BACKGROUND = "Button.background";
	private static final Color COLOR = UIManager.getColor(BUTTON_BACKGROUND);
	private Dimension labelSize = new Dimension(640,480);//earlier 600, 500
	public VideoCapture cam;
	private BufferedImage bimage;
	private BufferedImage bimage_w;
	private int zoom_value = 1;
	private IMediaWriter writer;
	private int displayLabelWidth = 1;
	private int displayLabelHeight = 1;
	public Mat mat = new Mat();
	public ImageIcon icon = new ImageIcon("images/camera.jpg");
	public JLabel label = new JLabel(icon);
	private long startTime = System.nanoTime();
	private boolean exit_playing = false;
	private boolean playing = false;
	private int FRAMES_DELAY = 20;

	public JFrame jfr = new JFrame("Interface Developed by GyanData Private Ltd.");
	JButton live = new JButton("Live Feed");
	JButton capture = new JButton("Capture Image");
	JButton up = new JButton("Tilt Up");
	JButton down = new JButton("Tilt Down");
	JButton left = new JButton("Pan Left");
	JButton right = new JButton("Pan Right");
	JButton save = new JButton("Record Video");
	JButton play = new JButton("Play Video");
	JButton pause = new JButton("Pause Video");
	JButton stop = new JButton("Stop");
	boolean blive = true;
	boolean bcapture = false;
	boolean bsave = false;
	boolean bpause = false;
	boolean bstop = false;
	boolean bplay = false;
	private JSlider zoom = new JSlider(JSlider.VERTICAL, 1, 5, 1);
	private int zoom_level;
	private JPanel compass;

	final private BufferedImage dial = ImageIO.read(new File(
			"/home/gyandata/Desktop/compass.png"));// Image for the compass
	public String playfilename;// File to be played
	public String savefilename;// Filename for saving the video

	public Gui_Complete() throws IOException {
		displayGui();// creates the GUI window with all the buttons.
		System.out.println("displayGui in constructor,done");
		cam = setCamera(cam);
		// opens the camera
		thread_live();
		// Live feed keeps running in the background.
		// System.out.println("label's size" + label.getSize());
		displayLabelWidth = label.getWidth();
		displayLabelHeight = label.getHeight();
		// System.out.println("label size:" + displayLabelWidth + "  "+
		// displayLabelHeight);
	}

	private VideoCapture setCamera(VideoCapture cam1) {// opens the camera
		cam1 = new VideoCapture(1);// '1'-corresponds to USB camera
									// (default)
		if (cam1.isOpened()) {
			System.out.println("Camera opened : 1-usbcam");// default camera
															// opened
		} else {
			System.out
					.println("1 : Camera not opened.. trying '0'(laptop cam)..");
			cam1 = new VideoCapture(0);// '0'-corresponds to laptop cam
			if (cam1.isOpened()) {
				System.out.println("Camera opened : 0(laptop cam)");
			} else {
				System.out.println("Camera not opened");// None of the
														// cameras are
														// opened
			}
		}
		System.out.println("leaving setcamera");
		return cam1;// camera object returned to Gui's constructor
	}

	// method to convert image type picture to Buffered image
	public static BufferedImage toBufferedImage(Image img) {
		// if type of img is bufferedimage, return.
		if (img instanceof BufferedImage) {
			return (BufferedImage) img;
		}
		// creates a bufferedimage to hold the created Bufferedimage
		BufferedImage bimage = new BufferedImage(img.getWidth(null),
				img.getHeight(null), BufferedImage.TYPE_INT_ARGB);
		// creates a Graphics2D and draws the matrix as a buffered image
		Graphics2D bGr = bimage.createGraphics();
		bGr.drawImage(img, 0, 0, null);
		// clear memory
		bGr.dispose();
		// return the converted Bufferedimage
		return bimage;
	}

	// converts Matrix type picture to BufferedImage
	public BufferedImage toBufferedImage(Mat m) {
		int type = BufferedImage.TYPE_BYTE_GRAY;
		if (m.channels() > 1) {
			type = BufferedImage.TYPE_3BYTE_BGR;
		}
		// set the required buffersize to hold the bytes
		int bufferSize = m.channels() * m.cols() * m.rows();
		byte[] b = new byte[bufferSize];
		m.get(0, 0, b); // get all the pixels
		BufferedImage image = new BufferedImage(m.cols(), m.rows(), type);
		// convert
		final byte[] targetPixels = ((DataBufferByte) image.getRaster()
				.getDataBuffer()).getData();
		System.arraycopy(b, 0, targetPixels, 0, b.length);
		return image;

	}

	// if 'zoom in' is active,
	// captures camera feed - one frame and updates on the Label as icon
	public void captureAndUpdateImage_zoom(VideoCapture cam) {
		// read a frame from camera
		cam.read(mat);
		System.out.println("cam read");
		// convert the matrix picture to Bufferedimage format
		bimage = toBufferedImage(mat);
		// get zoom value from slider
		zoom_value = zoom.getValue();
		// set icon size - the section of the frame to be enlarged and displayed
		displayLabelWidth = (int) bimage.getWidth() / zoom_value;
		displayLabelHeight = (int) bimage.getHeight() / zoom_value;
		// extract the subimage (this extracts from a corner (0,0)..)
		bimage = bimage
				.getSubimage(0, 0, displayLabelWidth, displayLabelHeight);
		// scale the extracted part of the frame to fit the label's width and
		// height
		bimage = toBufferedImage(bimage.getScaledInstance(label.getWidth(),
				label.getHeight(), Image.SCALE_SMOOTH));
		// System.out.println("bimage scaled..");
		// System.out.println("label:" + label.getWidth() + label.getHeight());
		// update the icon
		icon.getImage().flush();
		icon.setImage(bimage);
		bimage_w = bimage;
		System.out.println("bimage_w size inside zoom: "+bimage_w.getWidth()+bimage_w.getHeight());
		// set the icon as the label's icon
		label.setIcon(icon);
		System.out.println("going to updatelabel");
		// update the label for the change in icon to take effect
		label.updateUI();
		System.out.println("label updated");
	}

	// under no-zoom condition:
	// gets a frame, extracts a part, enlarges the subframe and updates it as
	// label's icon
	public void captureAndUpdateImage(VideoCapture cam) {
		// capture a frame
		cam.read(mat);
		System.out.println("cam read");
		// convert to buffered image
		bimage = toBufferedImage(mat);
		// update icon
		icon.getImage().flush();
		icon.setImage(bimage);
		bimage_w = bimage;
		System.out.println("bimage_w size without zoom: "+bimage_w.getWidth()+bimage_w.getHeight());
		// update label's icon
		label.setIcon(icon);
		System.out.println("going to updatelabel");
		// update label for the icon change to take effect
		label.updateUI();
		System.out.println("label updated");
	}

	// this thread runs in the background always.
	// updates frame when livefeed / capture image / savevideo is required.
	// during other time as like, playing video-this runs, but does nothing.
	public void thread_live() {
		// create a new thread
		Thread Tlive = new Thread() {
			public void run() {
				System.out.println("Tlive thread started");
				while (true) {
					// System.out.println("in Tlive");
					// depending on zoom level , call the appropriate camera
					// frame to label icon update function
					if (blive) {
						if (zoom.getValue() != 1) {
							captureAndUpdateImage_zoom(cam);
						} else {
							captureAndUpdateImage(cam);
						}
						// if capture is needed, write the frame to a file in
						// the system
						if (bcapture) {
							captureImage();
							bcapture = false;
						}
					}
				}// end of while loop
			}// end of run method
		};// end of thread
			// start the thread
		Tlive.start();
	}

	// when
	Thread Tsave = new Thread() {
		public void run() {
			while (!bstop) {
				System.out.println("in Tsave");
				writer.encodeVideo(0, bimage_w, System.nanoTime() - startTime,
						TimeUnit.NANOSECONDS);
				try {
					sleep(FRAMES_DELAY);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			bstop = false;
			writer.close();
		}
	};

	public void captureImage() {
		System.out.println("in capture image");
		this.cam.read(mat);
		Highgui.imwrite("/home/gyandata/Desktop/cameratest.jpg", mat);
		System.out.println("file written");
		bimage = toBufferedImage(mat);
		icon.getImage().flush();
		icon.setImage(bimage);
		label.setIcon(icon);
		label.updateUI();
		System.out.println("image update");
	}

	public JPanel drawCompass() {
		int w = dial.getWidth();
		int h = dial.getHeight();
		int step = w / 3;
		JPanel p = new JPanel(new GridLayout(3, 3));
		p.setOpaque(false);
		int count = 0;
		for (int ii = 0; ii < w; ii += step) {
			for (int jj = 0; jj < h; jj += step) {
				Image icon = dial.getSubimage(jj, ii, step, step);
				if (count % 2 == 1) {
					JButton button = new JButton(new ImageIcon(icon));

					button.setBorder(null);

					BufferedImage iconPressed = new BufferedImage(step, step,
							BufferedImage.TYPE_INT_ARGB);
					Graphics g = iconPressed.getGraphics();
					g.drawImage(icon, 0, 0, p);
					g.setColor(Color.gray);
					g.drawRoundRect(0, 0, iconPressed.getWidth(p) - 1,
							iconPressed.getHeight(p) - 1, 12, 12);
					g.dispose();
					button.setPressedIcon(new ImageIcon(iconPressed));

					button.setContentAreaFilled(false);

					if (count == 1) {
						button.setActionCommand("u");
					}
					if (count == 3) {
						button.setActionCommand("l");
					}
					if (count == 5) {
						button.setActionCommand("r");
					}
					if (count == 7) {
						button.setActionCommand("d");
					}
					button.addActionListener(new ActionListener() {
						@Override
						public void actionPerformed(ActionEvent ae) {
							System.out.println(ae.getActionCommand());
						}
					});

					p.add(button);
				} else {
					JLabel label = new JLabel(new ImageIcon(icon));
					p.add(label);
				}
				count++;
			}
		}
		return p;
	}

	public void displayGui() throws IOException {
		label.setMaximumSize(labelSize);
		label.setMinimumSize(labelSize);
		label.setPreferredSize(labelSize);

		zoom.setPaintTicks(true);
		zoom.setPaintLabels(true);
		zoom.setSnapToTicks(true);

		System.out.println("adding compass");
		compass = drawCompass();

		JPanel jpf = new JPanel();

		FlowLayout fln = new FlowLayout();
		JPanel jpf1 = new JPanel();
		jpf1.setLayout(fln);

		GridLayout glc = new GridLayout(2, 3);
		JPanel jpc = new JPanel();
		jpc.setLayout(glc);

		live.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				System.out.println("In live-action");
				thread_live();
				blive = true;
			}
		});

		capture.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				bcapture = true;
			}
		});

		save.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				System.out.println("save");
				bsave = true;
				Dimension screenBounds = Toolkit.getDefaultToolkit()
						.getScreenSize();

				// String outputFilename = "/home/gyandata/Desktop/saving2.mp4";
				JFileChooser saveFileChooser = new JFileChooser();
				int returnval = saveFileChooser.showSaveDialog(jfr);
				if (returnval == JFileChooser.APPROVE_OPTION) {
					File filetosave = saveFileChooser.getSelectedFile();
					savefilename = filetosave.getAbsolutePath();
					System.out.println("save :" + savefilename);
				}
				writer = ToolFactory.makeWriter(savefilename);
				writer.addVideoStream(0, 0, ICodec.ID.CODEC_ID_MPEG4,
						screenBounds.width, screenBounds.height);//.W/2   .H/2
				Tsave.start();
			}
		});

		play.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				System.out.println("play video");
				bplay = true;
				blive = false;
				if (!playing) {
					new Thread(new Runnable() {

						@Override
						public void run() {
							// videoplayer.setLabel(label);
							label = play();

							label.updateUI();
							blive = true;
							bplay = false;

						}
					}).start();
					playing = true;
				}

			}
		});

		pause.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				System.out.println("Pause");
				bpause = true;
				blive = false;
			}

		});

		stop.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				System.out.println("stop");
				bstop = true;
				bplay = false;
				bsave = false;
				blive = true;
				playing = false;
			}
		});

		zoom.addMouseMotionListener(new MouseMotionAdapter() {
			@Override
			public void mouseDragged(MouseEvent e) {
				System.out.println("zoom moved");
				zoom_level = zoom.getValue();
				System.out.println("zoom level is :" + zoom_level);
			}
		});

		live.setBackground(COLOR);
		capture.setBackground(COLOR);
		save.setBackground(COLOR);
		play.setBackground(COLOR);
		pause.setBackground(COLOR);
		stop.setBackground(COLOR);
//adding buttons to the panel (jpc)
		jpc.add(live);
		jpc.add(capture);
		jpc.add(save);
		jpc.add(play);
		jpc.add(pause);
		jpc.add(stop);
//adding label(for video) and slider to the panel (jpf1)
		jpf1.add(label);
		jpf1.add(zoom);
//setting layout for the overall JFrame-jfr
		GroupLayout groupLayout = new GroupLayout(jpf);

		groupLayout.setHorizontalGroup(groupLayout
				.createParallelGroup()
				.addComponent(jpf1)
				.addGroup(
						groupLayout.createSequentialGroup()
								.addComponent(jpc, 600, 600, 600)
								.addComponent(compass, 100, 100, 100)));
		groupLayout.setVerticalGroup(groupLayout
				.createSequentialGroup()
				.addComponent(jpf1, 500, 500, 500)
				.addGroup(
						groupLayout.createParallelGroup()
								.addComponent(jpc, 150, 150, 150)
								.addComponent(compass, 100, 100, 100)));

		jpf.setLayout(groupLayout);

		System.out.println("added compass");
//setting other visual parameters for the JFrame - jfr
		jfr.setSize(750, 700);
		jfr.setContentPane(jpf);
		jfr.setDefaultCloseOperation(jfr.EXIT_ON_CLOSE);
		jfr.setResizable(false);
		jfr.setVisible(true);
		System.out.println("display");
	}

//when play button is clicked:
	public JLabel play() {
	//choose the file to the played using JFileChooser
		final JFileChooser fileDialog = new JFileChooser();
		System.out.println("going to choose file to play");
		int returnVal = fileDialog.showOpenDialog(jfr);
		if (returnVal == JFileChooser.APPROVE_OPTION) {
			java.io.File file = fileDialog.getSelectedFile();
			//get complete path and store in a string variable (playfilename) 
			playfilename = file.getAbsolutePath();
			System.out.println("file chosen " + playfilename);
		} else {
			System.out.println("no file selected");
		}
//create an instance of  Imediareader class for viewing the 'playfilename' video 
		IMediaReader reader = ToolFactory.makeReader(playfilename);
		reader.setBufferedImageTypeToGenerate(BufferedImage.TYPE_3BYTE_BGR);
		MediaListenerAdapter adapter = new MediaListenerAdapter() {
			@Override
			public void onVideoPicture(IVideoPictureEvent event) {
				setImage((BufferedImage) event.getImage());
			}
		};
		// creates an internal thread for displaying the video on the label
		reader.addListener(adapter);
		while ((reader.readPacket() == null) && (!exit_playing))
			do {
				blive = false;
				if (bstop) {
					System.out.println("breaking because of stop");
					exit_playing = true;
					bstop = false;
					break;
				}
				if (bpause) {
					pause.setText("Resume Video");
					bpause = false;
					bstop = false;
					System.out.println("bpause is true");
					while (!bpause) {// loop until Resume is pressed..
						System.out
								.println("in while checking for bpause/bstop");
						try {

							java.lang.Thread.sleep(FRAMES_DELAY);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
						if (bstop) {
							System.out.println("exit-stop pressed");
							exit_playing = true;
							bstop = false;
						}
					}// loop for resume ends..
					bpause = false;
				}
				pause.setText("Pause Video");
			} while (false);
		// pause.setText("Pause");
		blive = true;
		playing = false;
		exit_playing = false;
		// thread_live();
		return label;
	}

	@SuppressWarnings("serial")
	Image image;
//update each and every frame of the video onto the 
	public void setImage(final Image image) {
		this.image = image;
		// System.out.println("icon is set to : " + icon);
		icon.setImage(image);

		label.setIcon(icon);
		try {
			java.lang.Thread.sleep(FRAMES_DELAY);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		label.validate();
		label.repaint();
		label.updateUI();
		System.out.println("updated labelinframe");

	}

	public synchronized void paint(Graphics g) {
		if (image != null) {
			g.drawImage(image, 0, 0, null);
		}
	}	

	public static void main(String[] args) throws IOException {
		System.loadLibrary(Core.NATIVE_LIBRARY_NAME);
		Gui_Complete g = new Gui_Complete();
	}

}
