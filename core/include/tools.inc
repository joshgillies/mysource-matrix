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
* $Id: tools.inc,v 1.1 2005/03/16 04:53:07 ndvries Exp $
*
*/


/**
* Tools
*
* Purpose
*
*    Provides management and access to system wide tools for
*    improving productivity. Modeled off the config system
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class Tools extends MySource_Object
{
	/**
	* Manages calling display and processing of tools
	*
	* @access public
	*/
    function paintBackend(&$backend)
	{
		$o = &$backend->out;
		$o->setHeading('System Tools', '<script language="JavaScript" type="text/javascript">sq_print_icon("'.$o->filesPath('/images/icons/header/whereami.png').'", "20", "20", "HIPO Icon");</script>');

		// the user has pressed commit for a specific tool
		if (isset($_REQUEST['committed_tool_type_code'])) {
			$this->_processTool($o);
			return;
		}

		// the user has selected a tool, lets call its interface
		if (isset($_REQUEST['tool_type_code'])) {
			$this->_paintTool($o);
			return;
		}

		// At this point, the user just wants to view a list of the available tools
		$o->openSection('System Tools');
		$o->openField('&nbsp;');
			$tools = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('tool');

			if (count($tools) > 0) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th>Name</th>
						<th>Version</th>
						<th>Description</th>
					</tr>
				<?php
				foreach ($tools as $tool) {
					$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($tool);
					$backend_url = $backend->getBackendUrl('main');
					?>
					<tr>
						<td class="sq-backend-table-cell"><a href="<?php echo $backend_url; ?>&backend_section=tools&tool_type_code=<?php echo $tool_info['type_code']; ?>"><?php echo $tool_info['name']; ?></a></td>
						<td class="sq-backend-table-cell"><?php echo $tool_info['version']; ?></td>
						<td class="sq-backend-table-cell"><?php echo $tool_info['description']; ?></td>
					</tr>
					<?php
				}
				?>
				</table>
				<?php
			}
			else {
				echo 'There are no tools installed in your system.';
			}
		$o->closeField();
		$o->closeSection();


	}//end paintBackend()


	/**
	* Calls a tools getInterface function
	*
	* @access private
	*/
	function _paintTool(&$o)
	{
		$type_code = $_REQUEST['tool_type_code'];
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($type_code)) {
			$am->includeAsset($type_code);
			eval('$content = '.$type_code.'::paintTool(&$o, $type_code, $type_code);');

			$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			$o->setHeading('System Tools: '.$tool_info['name'], '<script language="JavaScript" type="text/javascript">sq_print_icon("'.$o->filesPath('/images/icons/header/asset_tree.png').'", "20", "20", "HIPO Icon");</script>');
			echo $content;
		}

		return;


	}//end _paintTool()


	/**
	* Calls a tools processInterface function
	*
	* @access private
	*/
	function _processTool(&$o)
	{
		$type_code = $_REQUEST['committed_tool_type_code'];
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($type_code)) {
			$am->includeAsset($type_code);
			eval('$content = '.$type_code.'::processTool(&$o, $type_code);');
			echo $content;
		}

		return;
	}//end _processTool()


}//end class

?>
