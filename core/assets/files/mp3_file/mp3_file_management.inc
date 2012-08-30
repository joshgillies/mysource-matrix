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
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* MP3_File_Management
*
* @author  Nicholas Hubbard <nic@zedsaid.com>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class MP3_File_Management extends Asset_Management
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
					 	'extract_id3'			=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'boolean',
                                                    'default'       =>  FALSE,
                                                    'is_admin'      => '1',
													'parameters'	=> Array('allow_empty' => FALSE),
                                                    'description'   => 'When set to TRUE in edit interface, will use getID3() to extract ID3 information from the MP3 file.',
                                                   ),
					 	'write_mp3'				=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'boolean',
                                                    'default'       =>  FALSE,
                                                    'is_admin'      => '1',
													'parameters'	=> Array('allow_empty' => FALSE),
                                                    'description'   => 'When set to TRUE in edit interface, will use getID3() to write info to the MP3 file.',
                                                   ),
					 	'album'					=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'text',
                                                    'default'       => '',
                                                    'is_admin'      => '0',
                                                    'description'   => 'Name of album',
													'is_contextable'	=> TRUE,
                                                   ),
					 	'artist'				=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'text',
                                                    'default'       => '',
                                                    'is_admin'      => '0',
                                                    'description'   => 'Name of artist',
													'is_contextable'	=> TRUE,
                                                   ),
					 	'track'					=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'text',
                                                    'default'       => '',
                                                    'is_admin'      => '0',
                                                    'description'   => 'Track number',
                                                   ),
					 	'song'					=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'text',
                                                    'default'       => '',
                                                    'is_admin'      => '0',
                                                    'description'   => 'Name of track',
													'is_contextable'	=> TRUE,
                                                   ),
					 	'year'					=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'text',
                                                    'default'       => '',
                                                    'is_admin'      => '0',
                                                    'description'   => 'Year that track was created',
                                                   ),
					 	'bitrate'				=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'float',
                                                    'default'       => 0,
                                                    'is_admin'      => '0',
                                                    'description'   => 'Bitrate of encoded file',
                                                   ),
					 	'length'				=> Array(
						   							'added'         => '0.1',
						   							'type'          => 'duration',
						   							'default'       => 0,
						   							'is_admin'      => '0',
						   							'description'   => 'Length of audio file',
						   							),
					  	'genre'					=> Array(
						   							'added'         => '0.1',
						   							'type'          => 'text',
						   							'default'       => '',
						   							'is_admin'      => '0',
						   							'description'   => 'Genre of this audio file',
						   							),
					  	'samplerate'			=> Array(
						   							'added'         => '0.1',
						   							'type'          => 'float',
						   							'default'       => 0,
						   							'is_admin'      => '0',
						   							'description'   => 'Sample Rate of this audio file',
						   							),
					  	'lyrics'				=> Array(
						   							'added'         => '0.1',
						   							'type'          => 'text',
						   							'default'       => '',
						   							'is_admin'      => '0',
						   							'description'   => 'Lyrics for this audio file',
													'is_contextable'	=> TRUE,
						   							),
					 	'composer'				=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'text',
                                                    'default'       => '',
                                                    'is_admin'      => '0',
                                                    'description'   => 'Composer of this audio file',
                                                   ),
					 	'copyright'				=> Array(
                                                    'added'         => '0.1',
                                                    'type'          => 'text',
                                                    'default'       => '',
                                                    'is_admin'      => '0',
                                                    'description'   => 'Copyright',
                                                   ),
					  	'comments'				=> Array(
						   							'added'         => '0.1',
						   							'type'          => 'text',
						   							'default'       => '',
						   							'is_admin'      => '0',
						   							'description'   => 'Comments for this audio file',
													'is_contextable'	=> TRUE,
						   							),
												   
					);//end array

	}// end constructor


}// end class
?>
