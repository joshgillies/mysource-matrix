<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: report_management.inc,v 1.6 2005/03/21 02:51:54 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Report_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['name']['added']   = '0.2';
		$this->vars['name']['type']    = 'text';
		$this->vars['name']['default'] = '';

		$this->vars['generated_date']['added']   = '0.1';
		$this->vars['generated_date']['type']    = 'text';
		$this->vars['generated_date']['default'] = '-- Never --';

		$this->vars['generated_user']['added']   = '0.1';
		$this->vars['generated_user']['type']    = 'text';
		$this->vars['generated_user']['default'] = '';

		$this->vars['html']['added']       = '0.3';
		$this->vars['html']['type']        = 'text';
		$this->vars['html']['default']     = 'Front end component has not been generated for this report';
		$this->vars['html']['description'] = 'This is a default location of the generated report.';

	}//end constructor


}//end class

?>