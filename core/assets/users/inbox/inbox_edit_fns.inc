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
* $Id: inbox_edit_fns.inc,v 1.36 2005/02/07 22:36:23 dbaranovskiy Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Inbox_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.36 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Inbox_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Inbox_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens = Array(	'details'		=> Array(
															'name'			=> 'Inbox',
															'use_tab'		=> true,
															'force_unlock'	=> false,
															'lock_type'		=> 'mail',
															),
									);

	}//end constructor


	/**
	* Prints the message with the passed ID
	*
	* @param int	$messageid	the ID of the message to print
	*
	* @return boolean
	* @access public
	*/
	function paintMessage($messageid)
	{
		// get the message we are printing
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$message = $ms->getMessageById($messageid);

		// mark the message as read
		if ($message->status == SQ_MSG_UNREAD) {
			$message->updateStatus(SQ_MSG_READ);
		}

		return $message->printBody();

	}//end paintMessage()


	/**
	* Prints the inbox interface
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMail(&$asset, &$o, $prefix)
	{
		$user = &$asset->_getUser();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, null, Array(SQ_MSG_UNREAD, SQ_MSG_READ), Array(), null, null, 'name');
		$write_access = $asset->writeAccess('mail');

		if (empty($messages)) {
			echo 'You have no messages';
		} else {
			$allread = true;
			$num_messages = count($messages);
			for ($i = 0; $i < $num_messages; $i++) {
				$data = $messages[$i];
				if ($data['status'] == SQ_MSG_UNREAD) {
					$allread = false;
					break;
				}
			}
			?>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
			<script type="text/javascript">
			//<![CDATA[
				var Bodies = new Object();
				var prefix = "<?php echo $prefix; ?>";
				var timer = null;
				function showBody(msgid, send)
				{
					if (typeof send == "undefined") send = "";
					document.getElementById("sq_message_body" + send).innerHTML = Bodies[msgid];
					clearTimeout(timer);
					timer = setTimeout("msgRead('" + msgid + "')", 1000);
				}
				function msgRead(msgid)
				{
					var checkbox = document.getElementById(prefix + "_mark_as_read[" + msgid + "]");
					if (checkbox != null) {
						checkbox.checked = true;
						changeStatus(msgid);
					}
				}
				function readClick(obj)
				{
					var msgid = obj.id.substring(obj.id.indexOf("[") + 1, obj.id.indexOf("]"));
					changeStatus(msgid);
				}

				function changeStatus(msgid)
				{
					var checkbox1 = document.getElementById(prefix + "_mark_as_read[" + msgid + "]");
					var checkbox2 = document.getElementById(prefix + "_delete[" + msgid + "]");
					try {
						document.getElementById("letterIcon" + msgid).className = ((checkbox1.checked)?"":"un") + "read";
					}
					catch(e) {}

					var post = "process_form=1&am_form_submitted=1&asset_action=edit&backend_assetid=";
					post += document.getElementById("backend_assetid").value + "&";
					if (checkbox1 && checkbox1.checked) {
						post += prefix + "_mark_as_read[" + msgid + "]=on&";
					}

					if (checkbox2.checked) {
						post += prefix + "_delete[" + msgid + "]=on&";
					}

					post += prefix + "_messages[" + msgid + "]=on";

					var form = document.getElementById("main_form");
					JsHttpConnector.loadXMLDoc(form.action, post, form.method);
				}

				function changeAllStatuses(readordel)
				{
					var inputs = document.getElementsByTagName("INPUT");
					var post = "process_form=1&am_form_submitted=1&asset_action=edit&backend_assetid=";
					post += document.getElementById("backend_assetid").value + "&";

					for (var i = 0; i < inputs.length; i++) {
						if (readordel == "read") {
							if (inputs[i].id.indexOf("_mark_as_read[") < 0) continue;
						} else {
							if (inputs[i].id.indexOf("_delete[") < 0) continue;
						}
						inputs[i].checked = document.getElementById(prefix + "_select_all_" + ((readordel == "read")?"read":"delete")).checked;

						var msgid = inputs[i].id.substring(inputs[i].id.indexOf("[") + 1, inputs[i].id.indexOf("]"));

						var checkbox1 = document.getElementById(prefix + "_mark_as_read[" + msgid + "]");
						var checkbox2 = document.getElementById(prefix + "_delete[" + msgid + "]");
						if (checkbox1 != null) {
							document.getElementById("letterIcon" + msgid).className = ((checkbox1.checked)?"":"un") + "read";

							if (checkbox1.checked) {
								post += prefix + "_mark_as_read[" + msgid + "]=on&";
							}
						}

						if (checkbox2.checked) {
							post += prefix + "_delete[" + msgid + "]=on&";
						}

						post += prefix + "_messages[" + msgid + "]=on&";
					}
					var form = document.getElementById("main_form");
					JsHttpConnector.loadXMLDoc(form.action, post, form.method);
				}

				function Send()
				{
					var post = "process_form=1&am_form_submitted=1&asset_action=edit&backend_assetid=";
					post += document.getElementById("backend_assetid").value + "&";
					i = 0;
					for (var i = 0; i < cursel + 1; i++) {
						if (document.getElementById(prefix + "_To[" + i + "]") != null) {
							post += prefix + "_To[]=" + document.getElementById(prefix + "_To[" + i + "]").value + "&";
						}
					}
					post += prefix + "_Subject=" + escape(document.getElementById(prefix + "_Subject").value) + "&";
					post += prefix + "_Priority=" + priority + "&";
					post += prefix + "_Content=" + escape(document.getElementById(prefix + "_Content").value);
					var form = document.getElementById("main_form");
					JsHttpConnector.process = function()
					{
						document.getElementById("AlertBefore").style.display = "none";
						document.getElementById("AlertAfter").style.display = "block";
						setTimeout('document.location.href = document.location.href;', 250);
					}
					JsHttpConnector.loadXMLDoc(form.action, post, form.method);
					tabClick(2);
					document.getElementById("subsubAlert").style.display = "block";
					document.getElementById("subAlert").style.display = "block";
					document.getElementById("AlertBefore").style.display = "block";
				}


				var Selects = Array();
				var cursel = 1;
				function addUser(div)
				{
					var newdiv = document.createElement("DIV");
					Selects[cursel] = new TSelect("Selects[" + cursel + "]", prefix + "_To[" + (cursel + 1) + "]");
					Selects[cursel].searchfield = "longname";
					Selects[cursel].valuefield = "id";
					Selects[cursel].build = function(option, num)
					{
						option.text = this.results[num].longname;
						option.value = this.results[num].id;

					}
					newdiv.innerHTML = '<label id="label' + cursel + '" style="width: 10ex;float: left;">To:</label>' + Selects[cursel];
					var buttons = div.getElementsByTagName("BUTTON");
					var buttonMins = buttons[0].cloneNode(true);
					var buttonPlus = buttons[1].cloneNode(true);
					buttons[0].style.display = "";
					buttons[1].style.display = "none";
					newdiv.appendChild(buttonMins);
					newdiv.appendChild(buttonPlus);
					div.parentNode.appendChild(newdiv);
					cursel++;
				}


				function delUser(div)
				{
					var id = div.getElementsByTagName("LABEL")[0].id.substring(5);
					div.parentNode.removeChild(div);

				}

				function addUserSelector(select)
				{
					if (select.options.length == 2) {
						return;
					}
					var div = select.parentNode;
					var topdiv = div.parentNode;
					while (div.nextSibling != null) {
						topdiv.removeChild(div.nextSibling);
					}
					if (select.value == "") {
						return;
					}
					var id = select.id.substring(select.id.indexOf("[") + 1, select.id.indexOf("]"));
					id++;
					var newdiv = div.cloneNode(true);
					topdiv.appendChild(newdiv);
					var newselect = newdiv.getElementsByTagName("SELECT")[0];
					for (i = 0; i<newselect.options.length; i++) {
						if (newselect.options[i].value == select.value) {
							newselect.options[i] = null;
						}
					}
					newselect.selectedValue = 0;
					newselect.id = prefix + "_To[" + id + "]";
				}

				var priority = 3;
				function changePriority(div)
				{
					priority++;
					if (priority > 5) {
						priority = 1;
					}
					div.className = "prior" + priority;
					document.getElementById(prefix + "_Priority").value = priority;
				}

			//]]>
			</script>
			<script type="text/javascript">
			//<![CDATA[
				var curtab = 2;
				function tabClick(tab)
				{
					if (tab == 4) {
						//trash
						document.location.href = document.location.href + "&asset_ei_screen=trash";
					}
					document.getElementById("tab" + curtab).className = "tab";
					document.getElementById("div" + curtab).style.display = "none";
					curtab = tab;
					document.getElementById("tab" + curtab).className = "tab-selected";
					document.getElementById("div" + curtab).style.display = "block";
				}
				function init()
				{
					document.getElementById("tab" + curtab).className = "tab-selected";
					document.getElementById("div" + curtab).style.display = "block";
				}
			//]]>
			</script>
			<style type="text/css">
				#sq_message_body, #sq_message_bodysent {
					width: 95%;
					height: 250px;
					margin: 5px 0px;
					border: 1px solid black;
					white-space: pre;
					overflow: auto;
					padding: 1ex;
					font: 12pt "Lucida Grande", "Lucida Console", "Courier New";
				}
				.read {
					background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_read.png);
					background: expression('none');
					filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_read.png', sizingMethod='crop');
					height: 16px;
					width: 16px;
					float: left;
				}
				.unread {
					background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_unread.png);
					background: expression('none');
					filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_unread.png', sizingMethod='crop');
					height: 16px;
					width: 16px;
					float: left;
				}
				.prior1 {
					background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_1.png);
					background: expression('none');
					filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_1.png', sizingMethod='crop');
					height: 16px;
					width: 16px;
					float: left;
				}
				.prior2 {
					background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_2.png);
					background: expression('none');
					filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_2.png', sizingMethod='crop');
					height: 16px;
					width: 16px;
					float: left;
				}
				.prior3 {
					background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_3.png);
					background: expression('none');
					filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_3.png', sizingMethod='crop');
					height: 16px;
					width: 16px;
					float: left;
				}
				.prior4 {
					background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_4.png);
					background: expression('none');
					filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_4.png', sizingMethod='crop');
					height: 16px;
					width: 16px;
					float: left;
				}
				.prior5 {
					background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_5.png);
					background: expression('none');
					filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_5.png', sizingMethod='crop');
					height: 16px;
					width: 16px;
					float: left;
				}

				html, body {
					height: 100%;
				}
				#subsubAlert {
					position: absolute;
					top: 0;
					left: 0;
					width: 100%;
					height: 100%;
					display: none;
					background: #FFF;
					z-index: 200;
					opacity: 0;
					-moz-opacity: 0;
					filter: alpha(opacity=0);
				}
				#AlertBefore, #AlertAfter, #subAlert {
					position: absolute;
					width: 300px;
					height: 100px;
					top: 50%;
					left: 50%;
					margin-left: -150px;
					margin-top: -50px;
					z-index: 200;
					display: none;
				}
				#subAlert {
					background: #FFF;
					z-index: 200;
					opacity: 0.7;
					-moz-opacity: 0.7;
					filter: alpha(opacity=70);
				}
				#AlertBefore, #AlertAfter {
					border: solid 1px #000;
					background: none;
					z-index: 210;
					font: 10pt "Lucida Grande", Verdana, Arial, serif;
					text-align: center;
					padding: 25px;
					width: 250px;
					height: 50px;
				}
				#progressbar {
					position: absolute;
					width: 200px;
					height: 10px;
					font-size: 1px;
					top: 50%;
					left: 50%;
					margin-left: -100px;
					margin-top: -5px;
					background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/progressbar.gif);
					border: solid 1px #000;
				}
				/* tabs */

				#tabs_top ul {
					border-bottom: solid 1px #000;
					padding: 2px;
				}

				li.tab-selected, li.tab {
					display: inline;
					list-style-type: none;
					border-top: solid 1px #000;
					border-left: solid 1px #000;
					border-right: solid 1px #000;
					margin-left: 4px;
					padding: 2px;
					padding-top: 0;
				}

				li.tab {
					border-bottom: solid 1px #000;
					background: #DDD;
					cursor: pointer;
				}

				li.tab-selected {
					border-bottom: solid 1px #fff;
					cursor: default;
				}

				/* user selector */
				.results {
					position: absolute;
					display: none;
					font: 8pt "Lucida Grande", "Trebuchet MS", Tahoma, Arial;
					height: 200px;
				}
				.selector {
					width: 300px;
					font: 8pt "Lucida Grande", "Trebuchet MS", Tahoma, Arial;
				}

		</style>
		<div id="subsubAlert"></div>
		<div id="subAlert"></div>
		<div id="AlertBefore">
			Message is sending now&hellip;
			<div id="progressbar"></div>
		</div>
		<div id="AlertAfter">
			Message was sent successfully.<br />
		</div>
		<div id="tabs_top">
			<ul>
				<li class="tab" id="tab1" onclick="tabClick(this.id.substring(3))">Write message</li>
				<li class="tab" id="tab2" onclick="tabClick(this.id.substring(3))">Inbox</li>
				<li class="tab" id="tab3" onclick="tabClick(this.id.substring(3))">Sent</li>
				<li class="tab" id="tab4" onclick="tabClick(this.id.substring(3))">Trash</li>
			</ul>
		</div>
		<div id="tabs_container">
			<div id="div1" style="display: none">
				<?php
				if ($write_access) { ?>
					<div>
						<div>
							<label id="label0" style="width: 10ex;float: left;">To:</label>
							<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/TSelect.js"></script>
							<script type="text/javascript">
								var Clients = Array();
								TUsers = function(id, shortname, longname, imgpath)
								{
									this.id = id;
									this.shortname = shortname;
									this.longname = longname;
									this.imgpath = imgpath;
								}
								<?php
									$am = &$GLOBALS['SQ_SYSTEM']->am;
									$users = $am->getChildren(1, 'user', false);
									$users_count = count($users) - 1;
									foreach ($users as $id => $type) {
										$img_path = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($user->type());
										$user = &$am->getAsset($id);
										$name = $user->attr('first_name').' '.$user->attr('last_name');
										$shortname = $user->attr('username');
										?>
											Clients.push(new TUsers("<?php echo $id;?>", "<?php echo $shortname;?>", "<?php echo $name;?>", "<?php echo $img_path;?>"));
										<?php
									}
								?>
								Selects[0] = new TSelect("Selects[0]", prefix + "_To[0]");
								Selects[0].searchfield = "longname";
								Selects[0].valuefield = "id";
								Selects[0].build = function(option, num)
								{
									option.text = this.results[num].longname;
									option.value = this.results[num].id;

								}
								document.write(Selects[0]);
							</script>
							<button type="button" onclick="delUser(this.parentNode)" style="display: none;">&ndash;</button>
							<button type="button" onclick="addUser(this.parentNode)">+</button>
						</div>
					</div>
					<label style="width: 10ex;float: left;">Subject:</label>
					<input type="text" id="<?php echo $prefix; ?>_Subject" name="<?php echo $prefix; ?>_Subject" style="width:300px" />
					<label style="width: 10ex;float: left;">Priority:</label>
					<input type="hidden" id="<?php echo $prefix; ?>_Priority" />
					<button style="float:left;" type="button" onclick="changePriority(this.getElementsByTagName('DIV')[0]);"><div class="prior3"></div></button>
					<br />
					<textarea cols="3" rows="3" style="clear:both; width: 95%; height: 250px" id="<?php echo $prefix; ?>_Content" name="<?php echo $prefix; ?>_Content"></textarea>
					<button type="button" accesskey="s" onclick="Send()"><u>S</u>end</button>
				<?php } else { ?>
					<div style="text-align: center">Unlock inbox to send messages.</div>
				<?php } ?>
			</div>
			<div id="div2" style="display: none">
			<div style="overflow: auto; width: 95%; height: 250px; border: 1px solid black; margin: 5px 0px; ">
			<table border="0" cellspacing="0" cellpadding="1" width="100%">
			<tr>
				<td class="sq-backend-table-header">FROM</td>
				<td class="sq-backend-table-header">SUBJECT</td>
				<td class="sq-backend-table-header">SENT</td>
				<td class="sq-backend-table-header">
					<?php
					if ($write_access) {
						check_box($prefix.'_select_all_read', '1', $allread, "changeAllStatuses('read')");
					}
					?>
					READ
				</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">
						<?php check_box($prefix.'_select_all_delete', '1', false, "changeAllStatuses('del')"); ?>
						DELETE
					</td>
					<?php
				}
				?>
			</tr>
			<?php
			$alt = false;

			for ($i = 0; $i < $num_messages; $i++) {

				$data = $messages[$i];
				if ($data['type'] == 'hipo.sent') {
					continue;
				}

				if ($alt) $alt = false;
				else $alt = true;
				$class = 'sq-backend-table-cell'.(($alt) ? '-alt' : '');

				// work out the time of day icon
				if (date('A',$data['sent']) == 'AM') $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_day.png';
				else $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_night.png';

				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);
				$body = str_replace("\n", '<br />', $data['body']);
				$body = str_replace("\"", '&quot;', $body);
				?>
				<script type="text/javascript">
				//<![CDATA[
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<tr>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_'.$data['priority'].'.png', 16, 16) ?>
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="<?php echo $class ?>">
						<span onclick="showBody('<?php echo $data['msgid'];?>')" style="cursor: pointer">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/message_subject.png', 16, 16) ?>
						&nbsp;<?php echo $data['subject'];?>
						</span>
					</td>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php sq_print_icon($time_img, 16, 16) ?>
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php
						if ($asset->writeAccess('mail')) {
							check_box($prefix.'_mark_as_read['.$data['msgid'].']', '1', ($data['status'] == SQ_MSG_READ), 'readClick(this)', 'style="float:left;"');
						}
						?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" alt="" id="letterIcon<?php echo $data['msgid'];?>" class="<?php echo (($data['status'] == SQ_MSG_UNREAD) ? 'un' : ''); ?>read" />
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="<?php echo $class ?>" nowrap="nowrap">
							<?php check_box($prefix.'_delete['.$data['msgid'].']', '1', false, 'changeStatus('.$data['msgid'].')'); ?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}
			?>
			</table>
			</div>
			<div id="sq_message_body"></div>
			</div>
			<div id="div3" style="display: none">
			<div style="overflow: auto; width: 95%; height: 250px; border: 1px solid black; margin: 5px 0px; ">
			<table border="0" cellspacing="0" cellpadding="1" width="100%">
			<tr>
				<td class="sq-backend-table-header">TO</td>
				<td class="sq-backend-table-header">SUBJECT</td>
				<td class="sq-backend-table-header">SENT</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">
						<?php check_box($prefix.'_select_all_delete', '1', false, "changeAllStatuses('del')"); ?>
						DELETE
					</td>
					<?php
				}
				?>
			</tr>
			<?php
			$alt = false;

			for ($i = 0; $i < $num_messages; $i++) {

				$data = $messages[$i];
				if ($data['type'] != 'hipo.sent') {
					continue;
				}

				if ($alt) $alt = false;
				else $alt = true;
				$class = 'sq-backend-table-cell'.(($alt) ? '-alt' : '');

				// work out the time of day icon
				if (date('A',$data['sent']) == 'AM') $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_day.png';
				else $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_night.png';

				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);
				$body = preg_replace("/[\n\t\r]/im", '<br />', $data['body']);
				$body = str_replace("\"", '&quot;', $body);
				?>
				<script type="text/javascript">
				//<![CDATA[
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<tr>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<img src="<?php echo sq_web_path('lib').'/web/images/blank.gif'; ?>" alt="" style="background:url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_<?php echo $data['priority'];?>.png); background: expression('none');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_<?php echo $data['priority'];?>.png', sizingMethod='crop');width: 16px; height: 16px" />
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="<?php echo $class ?>">
						<span onclick="showBody('<?php echo $data['msgid'];?>','sent')" style="cursor: pointer">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/message_subject.png', 16, 16) ?>
						&nbsp;<?php echo $data['subject'];?>
						</span>
					</td>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php sq_print_icon($time_img, 16, 16) ?>
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="<?php echo $class ?>" nowrap="nowrap">
							<?php check_box($prefix.'_delete['.$data['msgid'].']', '1', false, 'changeStatus('.$data['msgid'].')'); ?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}
			?>
			</table>
			</div>
			<div id="sq_message_bodysent"></div>
			</div>
			<div id="div4" style="display: none"></div>

			<script type="text/javascript">
				<?php
					if (!empty($_REQUEST['tab'])) {
						echo 'curtab = "'.$_REQUEST['tab'].'"; ';
					}
				?>
				init();
			</script>
			<?php
		}

		return false;

	}//end paintMail()


	/**
	* Processes the inbox interface
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processMail(&$asset, &$o, $prefix)
	{
		$msgs   = (isset($_POST[$prefix.'_messages'])) ? array_keys($_POST[$prefix.'_messages']) : Array();
		$read   = (isset($_POST[$prefix.'_mark_as_read'])) ? array_keys($_POST[$prefix.'_mark_as_read']) : Array();
		$delete = (isset($_POST[$prefix.'_delete'])) ? array_keys($_POST[$prefix.'_delete']) : Array();

		$user = &$asset->_getUser();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id);

		// send the message
		if (isset($_POST[$prefix.'_To'])) {
			$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

			$message = $ms->newMessage($_POST[$prefix.'_To'], $_POST[$prefix.'_Subject'], $_POST[$prefix.'_Content'], 'hipo.some');
			$message->priority = $_POST[$prefix.'_Priority'];
			$message->send();

			foreach($_POST[$prefix.'_To'] as $key => $sender) {
				if (empty($sender)) {
					continue;
				}
				$message = $ms->newMessage(Array($userid), $_POST[$prefix.'_Subject'], $_POST[$prefix.'_Content'], 'hipo.sent');
				$message->from = $sender;
				$message->status = 'R';
				$message->priority = $_POST[$prefix.'_Priority'];
				$message->send();
			}

		}

		if (!empty($messages)) {
			foreach ($messages as $data) {
				if (!in_array($data['msgid'], $msgs)) continue;
				$message = $ms->getMessageById($data['msgid']);

				// change message status (read/unread)
				if (in_array($data['msgid'], $read) && $data['status'] != SQ_MSG_READ) {
					if (!$message->updateStatus(SQ_MSG_READ)) {
						trigger_error('Status of message '.$data['msgid'].' was not updated', E_USER_WARNING);
					}
				} else if (!in_array($data['msgid'], $read) && $data['status'] == SQ_MSG_READ) {
					if (!$message->updateStatus(SQ_MSG_UNREAD)) {
						trigger_error('Status of message '.$data['msgid'].' was not updated', E_USER_WARNING);
					}
				}

				// delete messages
				if (in_array($data['msgid'], $delete) && $data['status'] != SQ_MSG_DELETED) {
					if (!$message->updateStatus(SQ_MSG_DELETED)) {
						trigger_error('Message '.$data['msgid'].' was not deleted', E_USER_WARNING);
					}
				}
			}
		}

		$o->addOnLoad('if (parent.frames["sidenav"] && parent.frames["sidenav"].refresh_internal_messages) parent.frames["sidenav"].refresh_internal_messages();');

		return true;

	}//end processMail()


	/**
	* Prints the trash interface
	*
	* @param object File				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTrash(&$asset, &$o, $prefix)
	{
		$user = &$asset->_getUser();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id, null, Array(SQ_MSG_DELETED), Array(), null, null, 'name');
		$write_access = $asset->writeAccess('mail');

			?>
			<style type="text/css">
				#sq_message_body {
					width: 95%;
					height: 250px;
					margin: 5px 0px;
					border: 1px solid black;
					white-space: pre;
					overflow: auto;
					padding: 1ex;
					font: 12pt "Lucida Grande", "Lucida Console", "Courier New";
				}
				#tabs_top ul {
					border-bottom: solid 1px #000;
					padding: 2px;
				}

				li.tab-selected, li.tab {
					display: inline;
					list-style-type: none;
					border-top: solid 1px #000;
					border-left: solid 1px #000;
					border-right: solid 1px #000;
					margin-left: 4px;
					padding: 2px;
					padding-top: 0;
				}

				li.tab {
					border-bottom: solid 1px #000;
					background: #DDD;
					cursor: pointer;
				}

				li.tab-selected {
					border-bottom: solid 1px #fff;
					cursor: default;
				}
			</style>
			<script type="text/javascript" >
				function showMessageBody(messageid) {
					var iframe = document.getElementById('sq_message_body');
					if (iframe == null) return;

					var url='<?php echo $_SERVER['PHP_SELF']; ?>' + '?a=<?php echo $asset->id; ?>&msgid=' + messageid;
					iframe.src = url;
				}

				var Bodies = new Object();
				var prefix = "<?php echo $prefix; ?>";
				function showMessageBody(msgid)
				{
					document.getElementById("sq_message_body").innerHTML = Bodies[msgid];
				}
			</script>
			<script type="text/javascript">
			//<![CDATA[
				var curtab = 4;
				function tabClick(tab)
				{
					if (tab != 4) {
						var href = document.location.href;
						href = href.substring(0, href.indexOf("&asset_ei_screen"));
						href += "&tab=" + tab;
						document.location.href = href;
					}
				}
			//]]>
			</script>
		<div id="tabs_top">
			<ul>
				<li class="tab" id="tab1" onclick="tabClick(this.id.substring(3))">Write message</li>
				<li class="tab" id="tab2" onclick="tabClick(this.id.substring(3))">Inbox</li>
				<li class="tab" id="tab3" onclick="tabClick(this.id.substring(3))">Sent</li>
				<li class="tab-selected" id="tab4" onclick="tabClick(this.id.substring(3))">Trash</li>
			</ul>
		</div>
		<div id="tabs_container">
			<div id="div4" style="display: block">
		<?php
		if (empty($messages)) {
			echo 'Your trash is empty';
		} else {
		?>
			<div style="overflow: auto; width: 95%; height: 250px; border: 1px solid black; margin: 5px 0px; ">

			<table border="0" cellspacing="0" cellpadding="1" width="100%">
			<tr>
				<td class="sq-backend-table-header">FROM</td>
				<td class="sq-backend-table-header">SUBJECT</td>
				<td class="sq-backend-table-header">SENT</td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header">
						<?php check_box($prefix.'_select_all_recover', '1', false, "check_all(document.main_form, '{$prefix}_recover', this.checked);"); ?>
						RECOVER
					</td>
					<td class="sq-backend-table-header">
						<?php check_box($prefix.'_select_all_purge', '1', false, "check_all(document.main_form, '{$prefix}_purge', this.checked);"); ?>
						PURGE
					</td>
					<?php
				}
				?>
			</tr>
			<?php
			$alt = false;
			$num_messages = count($messages);

			for ($i = 0; $i < $num_messages; $i++) {

				$data = $messages[$i];

				if ($alt) $alt = false;
				else $alt = true;
				$class = 'sq-backend-table-cell'.(($alt) ? '-alt' : '');

				// work out the time of day icon
				if (date('A',$data['sent']) == 'AM') $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_day.png';
				else $time_img = sq_web_path('lib').'/web/images/icons/internal_message/clock_night.png';

				// work out the sender icon
				$sender_img = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data['type_code']);

				$body = preg_replace("/[\n\t\r]/im", '<br />', $data['body']);
				$body = str_replace("\"", '&quot;', $body);
				?>
				<script type="text/javascript">
				//<![CDATA[
					Bodies["<?php echo $data['msgid'];?>"] = "<?php echo $body; ?>";
				//]]>
				</script>
				<tr>
					<td class="<?php echo $class ?>" nowrap="nowrap">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_'.$data['priority'].'.png', 16, 16) ?>
						<?php sq_print_icon($sender_img, 16, 16) ?>
						<a name="msg_<?php echo $data['msgid'];?>"></a>
						&nbsp;<?php echo $data['from_name'];?>
						<?php hidden_field($prefix.'_messages['.$data['msgid'].']', '1'); ?>
					</td>
					<td class="<?php echo $class ?>">
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/message_subject.png', 16, 16) ?>
						&nbsp;<a class="sq-backend-table-cell" href="#" onClick="Javascript: showMessageBody('<?php echo $data['msgid'];?>'); return false;" style="text-decoration: none;"><?php echo $data['subject'];?></a>
					</td>
					<td class="<?php echo $class ?>" style="white-space: nowrap;">
						<?php sq_print_icon($time_img, 16, 16) ?>
						&nbsp;<?php echo $GLOBALS['SQ_SYSTEM']->datetime($data['sent']); ?>
					</td>

					<?php
					if ($write_access) {
						?>
						<td class="<?php echo $class ?>" nowrap>
							<?php
							if ($asset->writeAccess('mail')) check_box($prefix.'_recover['.$data['msgid'].']');
							else echo '&nbsp;';
							?>
						</td>
						<td class="<?php echo $class ?>" nowrap>
							<?php check_box($prefix.'_purge['.$data['msgid'].']'); ?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}
			?>
			</table>
			</div>
			<div id="sq_message_body"></div>
		<?php }
		?>
		</div>
		</div>
		<?php
		return true;

	}//end paintTrash()


	/**
	* Processes the trash interface
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processTrash(&$asset, &$o, $prefix)
	{
		$msgs    = (isset($_POST[$prefix.'_messages'])) ? array_keys($_POST[$prefix.'_messages']) : Array();
		$recover = (isset($_POST[$prefix.'_recover'])) ? array_keys($_POST[$prefix.'_recover']) : Array();
		$purge   = (isset($_POST[$prefix.'_purge'])) ? array_keys($_POST[$prefix.'_purge']) : Array();

		$user = &$asset->_getUser();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($user->id);

		if (!empty($recover)) {
			foreach ($recover as $messageid) {
				// recover messages to inbox
				$message = $ms->getMessageById($messageid);
				if (!$message->updateStatus(SQ_MSG_READ)) {
					trigger_error('Message '.$messageid.' was not recovered', E_USER_WARNING);
				}
			}
		}

		if (!empty($purge)) {
			foreach ($purge as $messageid) {
				// delete messsage FOREVER
				$message = $ms->getMessageById($messageid);
				if (!$message->delete()) {
					trigger_error('Message '.$messageid.' was not deleted', E_USER_WARNING);
				}
			}
		}

		return true;

	}//end processTrash()

}//end class

function sq_print_icon($path, $width, $height)
{
	echo '<img src="'.sq_web_path('lib').'/web/images/blank.gif" alt="" style="background:url('.$path.'); background: expression(\'none\');filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''.$path.'\', sizingMethod=\'crop\');width: '.$width.'px; height: '.$height.'px" />';
}

?>
