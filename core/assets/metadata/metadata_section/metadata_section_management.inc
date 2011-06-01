<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: metadata_section_management.inc,v 1.13 2008/10/15 03:09:09 bpearson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Section_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Section_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'		=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'frontend'	=> Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> TRUE,
										'parameters'	=> Array('allow_empty'	=> FALSE),
									   ),
						'restrict'	=> Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
									   ),
					  );

	}//end constructor


}//end class

?>
