/******************************************************************************
*  Copyright (C) 2011-2016 triTech Inc. All Rights Reserved.
*
*  See GPL-LICENSE.txt for copyright and license details.
*
*  This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation; either version 3 of the License, or (at your option) any later
* version. 
*
*  This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with
* this program. If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/******************************************************************************
 *
 *
 *	svc_logmsg.h
 *
 *	����	���O���b�Z�[�W
 *
 ******************************************************************************/
#ifndef	_SVC_LOGMSG_
#define	_SVC_LOGMSG_

#ifdef	ZZZ
#define	RDBCOMERR01 ""	/*"�v����M�G���[:%s"*/
#define	RDBCOMERR02 ""	/*"�������M�G���[:%s"*/
#define	RDBCOMERR03 ""	/*"�������s��:%s"*/
#define	RDBCOMERR04 ""	/*"�e�[�u���X�g�A�G���[:%s"*/
#define	RDBCOMERR05 ""	/*"�e�[�u���h���b�v�G���[:%s"*/
#define	RDBCOMERR06 ""	/*"�t�@�C�������G���[:%s"*/
#define	RDBCOMERR07 ""	/*"�t�@�C���I�[�v���G���[:%s"*/
#define	RDBCOMERR08 ""	/*"�t�@�C���폜�G���[:%s"*/
#define	RDBCOMERR09 ""	/*"�e�[�u������֎~:%s"*/
#define	RDBCOMERR10 ""	/*"�T�[�o��~���ŁA����ł��܂���:%s"*/

#define	MAINERR01 ""	/*"RPC�v���g�R���G���[:%s"*/
#define	MAINERR02 ""	/*"RPC�R�}���h�G���[:%s"*/
#define	MAINERR03 ""	/*"�N���C�A���g�Ƃ̒ʐM���ُ�ؒf����܂���(abort-code=%#x): %s"*/

#define	LINKERR01 ""	/*"�|�[�g�G���g���k�[��:E_RDB_CONFUSED"*/

#define	OPENLOG01 "[%s] is opened."	/*"�e�[�u�� %s �I�[�v������܂���"*/
#define	OPENLOG02 "[%s] is closed"	/*"�e�[�u�� %s �N���[�Y����܂���"*/
#define	OPENERR01 "open ERROR[%s]"	/*"�e�[�u���I�[�v���G���[:%s"*/
#define	OPENERR02 "close ERROR[%s]"	/*"�I�[�v�����[�U�N���[�Y�G���[:%s"*/

#define	CREATELOG01 ""	/*"�e�[�u�� %s ��������܂���"*/
#define	CREATELOG02 ""	/*"�e�[�u�� %s �폜����܂���"*/
#define	CREATEERR01 ""	/*"�e�[�u�����ɑ��݂��Ă��܂�:%s"*/
#define	CREATEERR02 ""	/*"�e�[�u���폜�ł��܂���:%s"*/

#define	DELETEERR01 ""	/*"���R�[�h���݂��Ă��܂���:%s"*/
#define	DELETEERR02 ""	/*"���R�[�h���b�N���ł�:%s"*/

#define	DIRECTERR01 DELETEERR01
#define	DIRECTERR02 DELETEERR02

#define	STOPLOG01 ""	/*"�T�[�o���ꎞ��~����܂���"*/
#define	STOPLOG02 ""	/*"�T�[�o���ĊJ����܂���"*/
#define	STOPERR01 ""	/*"�T�[�o�����ɒ�~���ł�:%s"*/
#define	STOPERR02 ""	/*"�T�[�o���ꎞ��~���邱�Ƃł��܂���:%s"*/
#define	STOPERR03 ""	/*"�T�[�o�����ɉ^�]���ł�:%s"*/

#define	INSERTERR01 ""	/*"�w�肷��A�C�e�����G���[:%s"*/
#define	INSERTERR02 ""	/*"�}�����R�[�h�͊��ɑ��݂��Ă��܂�:%s"*/
#define	INSERTERR03 ""	/*"�e�[�u���̊g���ł��܂���:%s"*/

#define	HOLDERR01 ""	/*"���郌�R�[�h�����b�N����Ă��܂�:%s"*/
#define	HOLDERR02 ""	/*"�e�[�u�����b�N���ł�:%s"*/
#define	HOLDERR03 ""	/*"�e�[�u���̃��b�N���[�U�͑��l�ł�:%s"*/

#define	EVENTERR01 ""	/*"�����C�x���g�N���X�w��G���[:%s"*/
#else
#define	RDBCOMERR01 "�v����M�G���[:%s"
#define	RDBCOMERR02 "�������M�G���[:%s"
#define	RDBCOMERR03 "�������s��:%s"
#define	RDBCOMERR04 "�e�[�u���X�g�A�G���[:%s"
#define	RDBCOMERR05 "�e�[�u���h���b�v�G���[:%s"
#define	RDBCOMERR06 "�t�@�C�������G���[:%s"
#define	RDBCOMERR07 "�t�@�C���I�[�v���G���[:%s"
#define	RDBCOMERR08 "�t�@�C���폜�G���[:%s"
#define	RDBCOMERR09 "�e�[�u������֎~:%s"
#define	RDBCOMERR10 "�T�[�o��~���ŁA����ł��܂���:%s"

#define	MAINERR01 "RPC�v���g�R���G���[:%s"
#define	MAINERR02 "RPC�R�}���h�G���[:%s"
#define	MAINERR03 "�N���C�A���g�Ƃ̒ʐM���ُ�ؒf����܂���(abort-code=%#x):%s"

#define	LINKERR01 "�|�[�g�G���g���k��:E_RDB_CONFUSED"

#define	OPENLOG01 "�e�[�u�� %s �I�[�v������܂���"
#define	OPENLOG02 "�e�[�u�� %s �N���[�Y����܂���"
#define	OPENERR01 "�e�[�u���I�[�v���G���[:%s"
#define	OPENERR02 "�I�[�v�����[�U�N���[�Y�G���[:%s"

#define	CREATELOG01 "�e�[�u�� %s ��������܂���"
#define	CREATELOG02 "�e�[�u�� %s �폜����܂���"
#define	CREATEERR01 "�e�[�u�����ɑ��݂��Ă��܂�:%s"
#define	CREATEERR02 "�e�[�u���폜�ł��܂���:%s"

#define	DELETEERR01 "���R�[�h���݂��Ă��܂���:%s"
#define	DELETEERR02 "���R�[�h���b�N���ł�:%s"

#define	DIRECTERR01 DELETEERR01
#define	DIRECTERR02 DELETEERR02

#define	STOPLOG01 "�T�[�o���ꎞ��~����܂���"
#define	STOPLOG02 "�T�[�o���ĊJ����܂���"
#define	STOPERR01 "�T�[�o�����ɒ�~���ł�:%s"
#define	STOPERR02 "�T�[�o���ꎞ��~���邱�Ƃł��܂���:%s"
#define	STOPERR03 "�T�[�o�����ɉ^�]���ł�:%s"

#define	INSERTERR01 "�w�肷��A�C�e�����G���[:%s"
#define	INSERTERR02 "�}�����R�[�h�͊��ɑ��݂��Ă��܂�:%s"
#define	INSERTERR03 "�e�[�u���̊g���ł��܂���:%s"

#define	HOLDERR01 "���郌�R�[�h�����b�N����Ă��܂�:%s"
#define	HOLDERR02 "�e�[�u�����b�N���ł�:%s"
#define	HOLDERR03 "�e�[�u���̃��b�N���[�U�͑��l�ł�:%s"

#define	EVENTERR01 "�����C�x���g�N���X�w��G���[:%s"
#endif

#endif /* _SVC_LOGMSG_ */
