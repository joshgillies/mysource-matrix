<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: report_links_management.inc,v 1.3 2006/03/15 02:56:23 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Links_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Report_Links_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Report_Links_Management(&$pm)
	{
		$this->vars['display_options'] = Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(
															'internal_section'	=> TRUE,
															'internal_broken'	=> TRUE,
															'internal_working'	=> TRUE,
															'external_section'	=> TRUE,
															'external_broken'	=> TRUE,
															'external_working'	=> TRUE,
															'email_section'		=> TRUE,
															'email_broken'		=> TRUE,
															'email_working'		=> TRUE,
														   ),
										 );

		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
