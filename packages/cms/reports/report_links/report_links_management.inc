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
* $Id: report_links_management.inc,v 1.6 2006/12/07 00:04:25 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Links_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
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
		$this->vars['report_broken_links'] = Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											 );
		$this->vars['report_info'] = Array(
										'added'		=> '0.2',
										'type'		=> 'text',
										'default'	=> '',
									 );
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
