<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: video_file_management.inc,v 1.4 2012/08/30 01:09:07 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Video_File_Management
*
* @author  Nicholas Hubbard <nic@zedsaid.com>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Video_File_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager relevant to this particular asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'extract_id3'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'is_admin'		=> '1',
											'parameters'	=> Array('allow_empty' => FALSE),
											'description'	=> 'When set to TRUE in edit interface, will use getID3() to extract ID3 information from the Video file.',
										   ),
						'file_format'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'File Format of this file',
										   ),
						'audio_codec'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Codec of the audio in this file',
										   ),
						'channels'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> 0,
											'is_admin'		=> '0',
											'description'	=> 'Number of channels of the audio in this file',
										   ),
						'channel_mode'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Channel Mode of the audio in this file',
										   ),
						'bitrate'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'float',
											'default'		=> 0,
											'is_admin'		=> '0',
											'description'	=> 'Bit Rate of the audio in this file',
											'parameters'	=> Array(
																'decimal_places'	=> 0,
															   ),
										   ),
						'sample_rate'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'float',
											'default'		=> 0,
											'is_admin'		=> '0',
											'description'	=> 'Sample Rate of the audio in this file',
											'parameters'	=> Array(
																'decimal_places'	=> 0,
															   ),
										   ),
						'video_codec'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Codec of the video in this file',
										   ),
						'width'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'float',
											'default'		=> 0,
											'is_admin'		=> '0',
											'description'	=> 'Width of the video',
											'parameters'	=> Array(
																'decimal_places'	=> 0,
															   ),
										   ),
						'height'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'float',
											'default'		=> 0,
											'is_admin'		=> '0',
											'description'	=> 'Height of the video',
											'parameters'	=> Array(
																'decimal_places'	=> 0,
															   ),
										   ),
						'fps'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'float',
											'default'		=> 0,
											'is_admin'		=> '0',
											'description'	=> 'Frames Per Second of the video',
											'parameters'	=> Array(
																'decimal_places'	=> 2,
															   ),
										   ),
						'length'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'duration',
											'default'		=> 0,
											'is_admin'		=> '0',
											'description'	=> 'Length of this file',
										   ),
						'video_title'	=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Title of the video',
										   ),
						'artist'		=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Artist of the video',
										   ),
						'album'			=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Album of the video',
										   ),
						'genre'			=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Genre of the video',
										   ),
						'year'			=> Array(
											'added'			=> '0.2',
											'type'			=> 'float',
											'default'		=> 0,
											'is_admin'		=> '0',
											'description'	=> 'Year the video was created',
											'parameters'	=> Array(
																'decimal_places'	=> 0,
															   ),
										   ),
						'copyright'		=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Copyright of the video',
										   ),
						'comments'		=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> '0',
											'description'	=> 'Description of the video',
										   ),
						'alt_asset'		=> Array(
											'added'			=> '0.2',
											'type'			=> 'assetid',
											'default'		=> '',
											'description'	=> 'Alternate Asset Reference for Accessibility',
											'update_fn'		=> '',
											'is_admin'		=> FALSE,
											),
					  );

	}//end constructor


}//end class
?>
