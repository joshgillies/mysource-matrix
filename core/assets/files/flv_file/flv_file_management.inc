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
* $Id: flv_file_management.inc,v 1.2 2012/08/30 01:09:07 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* FLV_File_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class FLV_File_Management extends Asset_Management
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
						 	'extract_metadata'	=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'boolean',
		                                                    'default'       => FALSE,
		                                                    'is_admin'      => TRUE,
															'parameters'	=> Array('allow_empty' => FALSE),
		                                                    'description'   => 'When set to TRUE in edit interface, will use getID3() to extract metadata from the FLV file',
                                                	    ),
						 	'duration'			=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'Duration of the FLV file in seconds',
	                                               		),
						 	'width'				=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'The width of the video in pixels',
                                                    	),
						 	'height'			=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'The height of the video in pixels',
                                                    	),
						 	'framerate'			=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'Number of frames per second',
	                                                    ),
						 	'videodatarate'		=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'Video bit rate in kilobits per second',
                                                    	),
						 	'videocodecid'		=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'The video codec ID used in the file',
	                                                    ),
						 	'audiodatarate'		=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'Audio bit rate in kilobits per second',
	                                                    ),
						 	'audiocodecid'		=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'The audio codec ID used in the file',
                                                    	),
						 	'audiosamplerate'	=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'The frequency at which the audio stream is replayed',
	                                                    ),
						 	'audiosamplesize'	=> Array(
		                                                    'added'         => '0.1',
		                                                    'type'          => 'float',
															'default'		=> 0,
															'is_admin'		=> FALSE,
		                                                    'description'   => 'The resolution of a single audio sample',
	                                                    ),
							'attributes'		=> Array(
															'added'		=> '0.1',
															'type'		=> 'serialise',
															'default'	=> Array(),
															'is_admin'		=> TRUE,
		                                                    'description'   => 'This attribute stores all the FLV metadata which are not kept in the above attributes',
													   ),
						   );
		
	}//end constructor


}//end class
?>