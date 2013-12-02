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
* $Id: tools.inc,v 1.13 2013/04/17 04:29:59 cupreti Exp $
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
* @version $Revision: 1.13 $
* @package MySource_Matrix
*/
class Tools extends MySource_Object
{


	/**
	* Manages calling display and processing of tools
	*
	* @param object	&$backend	Reference to backend object
	*
	* @return void
	* @access public
	*/
	function paintBackend(&$backend)
	{
		$o =& $backend->out;
		$o->setHeading(translate('system_tools'), sq_get_icon($o->filesPath('/images/icons/header/whereami.png'), '', '', 'System Tools'));
        $o->setPageTitle(translate('system_tools'));

		if (isset($_REQUEST['committed_tool_type_code'])) {
			// the user has pressed commit for a specific tool
			if (!$this->_processTool($o)) {
				// something went wrong - need to show the interface again
				$_REQUEST['tool_type_code'] = $_REQUEST['committed_tool_type_code'];
			}
		}

		// the user has selected a tool, lets call its interface
		if (isset($_REQUEST['tool_type_code'])) {
			$this->_paintTool($o);
			return;
		}

		// At this point, the user just wants to view a list of the available tools
		$o->openSection(translate('system_tools'));
		$o->openField('');
			$tools = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('tool');

			if (count($tools) > 0) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th><?php echo translate('name'); ?></th>
						<th><?php echo translate('description'); ?></th>
					</tr>
				<?php

				// Get the info for the each tool
				$tools_info = Array();
				foreach ($tools as $tool) {
					$tools_info[$tool] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($tool);
				}//end foreach

				// Sort the list by "name"
				uasort($tools_info, create_function('$a,$b', 'return strtolower($a["name"]) > strtolower($b["name"]);'));

				$backend_url = $backend->getBackendUrl('main');
				foreach($tools_info as $tool => $tool_info) {
					?>
					<tr>
						<td class="sq-backend-table-cell"><a href="<?php echo $backend_url; ?>&backend_section=tools&tool_type_code=<?php echo $tool; ?>"><?php echo $tool_info['name']; ?></a></td>
						<td class="sq-backend-table-cell"><?php echo $tool_info['description']; ?></td>
					</tr>
					<?php
				}
				?>
				</table>
				<?php
			} else {
				echo translate('tools_none_installed');
			}
		$o->closeField();
		$o->closeSection();

	}//end paintBackend()


	/**
	* Calls a tools getInterface function
	*
	* @param object	&$o	Reference to Outputter object
	*
	* @return void
	* @access private
	*/
	function _paintTool(&$o)
	{
		$type_code = $_REQUEST['tool_type_code'];
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($type_code)) {
			$am->includeAsset($type_code);
			$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			$o->setHeading('System Tools: '.$tool_info['name'], sq_get_icon($o->filesPath('/images/icons/header/asset_tree.png'), 20, 20, 'Tools'));
            $o->setPageTitle('System Tools: '.$tool_info['name']);
			call_user_func_array(Array($type_code, 'paintTool'), Array(&$o, $type_code));
			$o->addHiddenField('committed_tool_type_code', $type_code);
			$o->commitButton();
		}

	}//end _paintTool()


	/**
	* Calls a tools processInterface function
	*
	* @param object	&$o	Reference to Outputter object
	*
	* @return void
	* @access private
	*/
	function _processTool(&$o)
	{
		$type_code = $_REQUEST['committed_tool_type_code'];
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($type_code)) {
			$am->includeAsset($type_code);
			return call_user_func_array(Array($type_code, 'processTool'), Array(&$o, $type_code));
		}

		return FALSE;

	}//end _processTool()


}//end class

?>
