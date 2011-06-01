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
* $Id: bodycopy_container_management.inc,v 1.12 2007/12/04 05:35:34 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bodycopy_Container_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Container_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'description'	=> Array(
											'added'			=> '0.6',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Description for this bodycopy container',
											'is_admin'		=> FALSE,
										   ),
						'attributes'	=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'shadow_links'	=> Array(
											'added'			=> '0.3',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Array holding the assetids belonging to shadow assets that are used within the content. Needed because they cannot be notice linked like normal assets can.',
										   ),
					  );

	}//end constructor


}//end class

?>
