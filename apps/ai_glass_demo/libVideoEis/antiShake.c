/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. */


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_ipu.h"
#include "antiShake.h"

uint32_t get_branch_quu_trans4_distrect_gradual_joint_yv12_new(
    T_TRANS4_PAR* trans4_para_in[2],
    uint8_t src_img_id,
    uint8_t dst_img_id,
    uint8_t cmd_id_input,
    uint8_t branch_quu_buffer[] )
{
    E_IPU_STATUS status;

    int32_t scale_w;
    int32_t scale_h;
    int32_t crop_x;
    int32_t crop_y;
    int32_t crop_w;
    int32_t crop_h;
    int32_t rotate_w;
    int32_t rotate_h;
    int32_t cosR;
    int32_t sinR;
    int32_t blkNumH0;
    int32_t blkNumW0;
    int32_t blkNumT0;

    T_IPU_CMD_TRANS4_2* pcmdL;

    int32_t i;
    int32_t j;

    T_IPU_HW_RES hw_res;
    T_IPU_HW_CMD_TRANS4 branch_trans4 = { 0 };


    T_IPU_HW_FLAG flag = { 0 };
    uint16_t cmd_id;
    uint8_t img_id;
    uint32_t cmd_offset;
    uint32_t rsp_offset;
    uint32_t branch_offset;
    uint8_t* pbranch;
    uint32_t branch_length;
    uint32_t branch_num;
    uint32_t branch_trans4_offset[128] = { 0 };
    uint64_t* pcmd64;
    uint64_t* pbranch64;
    uint32_t cmd_offset8;
    uint32_t branch_offset8;

    FILE* fp;
    FILE* fp2;

    T_TRANS4_PAR trans4_para[3];//0, 1 are binocular images, 2 is blend image

    int32_t block_cnt = 0;

    for (i = 0; i < 2; i++)
    {
        memcpy(&trans4_para[i], trans4_para_in[i], sizeof(T_TRANS4_PAR));
        if (trans4_para_in[i]->dist_function == DISTRECT_NONE)
        {
            trans4_para[i].trans4_w = trans4_para_in[i]->src_w;
            trans4_para[i].trans4_h = trans4_para_in[i]->src_h;
        }
        else
        {
            trans4_para[i].trans4_w = trans4_para_in[i]->trans4_w;
            trans4_para[i].trans4_h = trans4_para_in[i]->trans4_h;
        }

        if (trans4_para_in[i]->crop_enable == 1)
        {
            trans4_para[i].crop_x = trans4_para_in[i]->crop_x;
            trans4_para[i].crop_y = trans4_para_in[i]->crop_y;
            trans4_para[i].crop_w = trans4_para_in[i]->crop_w;
            trans4_para[i].crop_h = trans4_para_in[i]->crop_h;
        }
        else
        {
            trans4_para[i].crop_x = 0;
            trans4_para[i].crop_y = 0;
            trans4_para[i].crop_w = trans4_para[i].trans4_w;
            trans4_para[i].crop_h = trans4_para[i].trans4_h;
        }

        if (trans4_para_in[i]->scale_enable == 1)
        {
            trans4_para[i].scale_w = trans4_para_in[i]->scale_w;
            trans4_para[i].scale_h = trans4_para_in[i]->scale_h;
        }
        else
        {
            trans4_para[i].scale_w = trans4_para[i].crop_w;
            trans4_para[i].scale_h = trans4_para[i].crop_h;
        }

        if (trans4_para_in[i]->rotate_enable == 0)
        {
            trans4_para[i].rotate = ROTATE_0;
            trans4_para[i].rotate_w = trans4_para[i].scale_w;
            trans4_para[i].rotate_h = trans4_para[i].scale_h;
        }
        else
        {
            trans4_para[i].rotate = trans4_para_in[i]->rotate;
            trans4_para[i].rotate_w = trans4_para_in[i]->rotate_w;
            trans4_para[i].rotate_h = trans4_para_in[i]->rotate_h;
        }

        trans4_para[i].dst_w = trans4_para[i].rotate_w;
        trans4_para[i].dst_h = trans4_para[i].rotate_h;
    }

    switch (trans4_para[0].rotate)
    {
    case ROTATE_0:
        cosR = 1;
        sinR = 0;
        break;
    case ROTATE_180:
        cosR = -1;
        sinR = 0;
        break;
    case ROTATE_90:
        cosR = 0;
        sinR = 1;
        break;
    case ROTATE_270:
        cosR = 0;
        sinR = -1;
        break;
    default:
        cosR = 1;
        sinR = 0;
        break;
    }

    blkNumW0 = (trans4_para[0].dst_w + trans4_para[0].block_w - 1) / trans4_para[0].block_w;
    blkNumH0 = (trans4_para[0].dst_h + trans4_para[0].block_h - 1) / trans4_para[0].block_h;
    blkNumT0 = blkNumW0 * blkNumH0;
    pcmdL = (T_IPU_CMD_TRANS4_2 *)malloc(blkNumT0 * sizeof(T_IPU_CMD_TRANS4));
    memset(pcmdL, 0, blkNumT0 * sizeof(T_IPU_CMD_TRANS4));

    //Parameter type inconsistency
    trans4_coordinate_change_2(
        ( T_IPU_CMD_TRANS4* )pcmdL,
        &trans4_para[0]
        );

    flag.pre_sync = 0;
    flag.post_sync = 1;
    flag.response = 1;
    flag.interrupt = 0;
    cmd_id = cmd_id_input;
    img_id = 0;
    cmd_offset = 0;
    rsp_offset = 0;
    branch_offset = 0;
    pbranch = branch_quu_buffer;

    branch_num = 0;
    branch_trans4_offset[branch_num] = branch_offset;
    branch_length = 0;


    block_cnt = trans4_para[0].block_cnt;
    for (block_cnt; block_cnt > 0; block_cnt -= 1)
    {
        cmd_id += 1;
        branch_trans4 = set_IPU_HW_CMD_TRANS4(&flag, cmd_id,
            dst_img_id, src_img_id,
            ( T_IPU_CMD_TRANS4 * )pcmdL + block_cnt - 1);
        memcpy(pbranch, &branch_trans4, sizeof(T_IPU_HW_CMD_TRANS4));
        pbranch += sizeof(T_IPU_HW_CMD_TRANS4);
        branch_offset += sizeof(T_IPU_HW_CMD_TRANS4);
        rsp_offset += sizeof(T_IPU_HW_RESP_FUNCTION);
        branch_length += sizeof(T_IPU_HW_CMD_TRANS4);

    }
    pbranch64 = (uint64_t*)branch_quu_buffer;
    cmd_offset8 = cmd_offset >> 3;
    branch_offset8 = branch_offset >> 3;

    free(pcmdL);
    return branch_length;
}

uint8_t set_IPU_HW_FLAG(T_IPU_HW_FLAG* flag)
{
    uint8_t val;

    val = ((flag->pre_sync & 0X1) << (IPU_CMD_FLAG_PRE_SYNC - 8)) |
        ((flag->post_sync & 0X1) << (IPU_CMD_FLAG_POST_SYNC - 8)) |
        ((flag->response & 0X1) << (IPU_CMD_FLAG_RESPONSE - 8)) |
        ((flag->interrupt & 0X1) << (IPU_CMD_FLAG_INTERRUPT - 8));

    return val;
}

T_IPU_HW_CMD_HEAD set_IPU_HW_CMD_HEAD(
    E_IPU_CMD_CODE code,
    T_IPU_HW_FLAG* flag,
    uint8_t cmd_id)
{
    T_IPU_HW_CMD_HEAD header = { 0 };

    header.code = (uint8_t)code;
    header.flags = set_IPU_HW_FLAG(flag);
    header.cmd_id = cmd_id;

    return header;
}

T_IPU_HW_CMD_TRANS4 set_IPU_HW_CMD_TRANS4(
    T_IPU_HW_FLAG* flag,
    uint16_t cmd_id,
    uint8_t dst_id,
    uint8_t src_id,
    T_IPU_CMD_TRANS4* cmd4)
{
    T_IPU_HW_CMD_TRANS4 cmd = { 0 };
    uint8_t* p;

    cmd.header = set_IPU_HW_CMD_HEAD(IPU_CMD_TRANS4, flag, cmd_id);
    cmd.dst_id = dst_id;
    cmd.src_id = src_id;
    cmd.blk_w = cmd4->out_res.w - 1;
    cmd.blk_h = cmd4->out_res.h - 1;
    cmd.dst_col = cmd4->out_pos.col;
    cmd.dst_row = cmd4->out_pos.row;
    cmd.weit_stepW2 = cmd4->weit_step.col_step2;
    cmd.weit_stepW3 = cmd4->weit_step.col_step3;
    cmd.weit_stepH2 = cmd4->weit_step.row_step2;
    cmd.weit_stepH3 = cmd4->weit_step.row_step3;
    p = (uint8_t*)(&cmd4->weit_step.row_step4);
    cmd.weit_stepH4[0] = p[0];
    cmd.weit_stepH4[1] = p[1];
    cmd.weit_stepH4[2] = p[2];
    p = (uint8_t*)(&cmd4->weit_step.row_step1);
    cmd.weit_stepH1[0] = p[0];
    cmd.weit_stepH1[1] = p[1];
    cmd.weit_stepH1[2] = p[2];
    p = (uint8_t*)(&cmd4->weit_step.col_step4);
    cmd.weit_stepW4[0] = p[0];
    cmd.weit_stepW4[1] = p[1];
    cmd.weit_stepW4[2] = p[2];
    p = (uint8_t*)(&cmd4->weit_step.col_step1);
    cmd.weit_stepW1[0] = p[0];
    cmd.weit_stepW1[1] = p[1];
    cmd.weit_stepW1[2] = p[2];

    return cmd;
}

int init_trans4_par( T_TRANS4_PAR *p_trans4_par, int src_w, int src_h, int dst_w, int dst_h )
{
    if( p_trans4_par == NULL ) {
        return -1;
    }
    memset( p_trans4_par, 0x00, sizeof( T_TRANS4_PAR ) );
#if 1
    p_trans4_par->block_w = TRANS_BLOCK;
    p_trans4_par->block_h = TRANS_BLOCK;
#else
    p_trans4_par->block_w = dst_w; //32 | 64 block size
    p_trans4_par->block_h = dst_h;
#endif
    p_trans4_par->src_w = src_w;
    p_trans4_par->src_h = src_h;

    p_trans4_par->start_x = 0;
    p_trans4_par->start_y = 0;
    p_trans4_par->dst_w = dst_w;
    p_trans4_par->dst_h = dst_h;
#if 1
    p_trans4_par->dist_function = DISTRECT_TRAN4_NEW;
#else
    p_trans4_par->dist_function = DISTRECT_NONE;
#endif
    p_trans4_par->trans4_w = dst_w;
    p_trans4_par->trans4_h = dst_h;

    p_trans4_par->block_cnt = 0;

    p_trans4_par->trans4_factor.perspective_transform_enable = true;
    return 0;
}

void trans4_coordinate_change_2(
    T_IPU_CMD_TRANS4* pcmd,
    T_TRANS4_PAR* trans4_para
    )
{
    double scale_r_w;
    double scale_r_h;
    double rot180;
    double cosRot;
    double sinRot;
    double normal_x, normal_y;

    int32_t blockNumW;
    int32_t blockNumH;

    int32_t i;
    int32_t j;
    int32_t k;
    T_COORD_S32 dst_pos[4];
    T_COORD_DOUBLE scale_pos[4];
    T_COORD_DOUBLE crop_pos[4];
    T_COORD_DOUBLE trans4_pos[4];
    T_COORD_DOUBLE src_pos[4];
    T_STEP_DOUBLE weitStep;

    int32_t block_w = trans4_para->block_w;
    int32_t block_h = trans4_para->block_h;
    int32_t src_w = trans4_para->src_w;
    int32_t src_h = trans4_para->src_h;
    int32_t dst_w = trans4_para->dst_w;
    int32_t dst_h = trans4_para->dst_h;
    int32_t trans4_w = trans4_para->trans4_w;
    int32_t trans4_h = trans4_para->trans4_h;
    int32_t crop_x = trans4_para->crop_x;
    int32_t crop_y = trans4_para->crop_y;
    int32_t crop_w = trans4_para->crop_w;
    int32_t crop_h = trans4_para->crop_h;
    int32_t scale_w = trans4_para->scale_w;
    int32_t scale_h = trans4_para->scale_h;
    int32_t rotate_w = trans4_para->rotate_w;
    int32_t rotate_h = trans4_para->rotate_h;
    int32_t start_x = trans4_para->start_x;
    int32_t start_y = trans4_para->start_y;
    int32_t move_x = trans4_para->move_x;
    int32_t move_y = trans4_para->move_y;
    int rotate = trans4_para->rotate;
    int32_t block_cnt = 0;

    if (trans4_para->scale_enable == 1)
    {
        scale_r_w = ((double)crop_w) / scale_w;
        scale_r_h = ((double)crop_h) / scale_h;
    }

    rot180 = M_PI180 * ((double)trans4_para->rotate * 90);
    cosRot = cos(rot180);
    sinRot = sin(rot180);

    blockNumW = (dst_w + block_w - 1) / block_w;
    blockNumH = (dst_h + block_h - 1) / block_h;

    block_cnt = 0;
    for (j = 0; j < blockNumH; j++)
    {
        for (i = 0; i < blockNumW; i++)
        {

            pcmd[block_cnt].pImgIn = NULL;
            pcmd[block_cnt].pImgOut = NULL;
            pcmd[block_cnt].out_pos.col = i * block_w + start_x;
            pcmd[block_cnt].out_pos.row = j * block_h + start_y;
            pcmd[block_cnt].out_res.h = (j == blockNumH - 1) ? (dst_h + start_y - pcmd[block_cnt].out_pos.row) : (block_h);
            pcmd[block_cnt].out_res.w = (i == blockNumW - 1) ? (dst_w + start_x - pcmd[block_cnt].out_pos.col) : (block_w);

            dst_pos[0].col = pcmd[block_cnt].out_pos.col - start_x - move_x;
            dst_pos[0].row = pcmd[block_cnt].out_pos.row - start_y - move_y;
            dst_pos[1].col = dst_pos[0].col + pcmd[block_cnt].out_res.w - 1;
            dst_pos[1].row = dst_pos[0].row;
            dst_pos[2].col = dst_pos[0].col;
            dst_pos[2].row = dst_pos[0].row + pcmd[block_cnt].out_res.h - 1;
            dst_pos[3].col = dst_pos[1].col;
            dst_pos[3].row = dst_pos[2].row;

            if (trans4_para->rotate_enable == 1)
            {
                normal_x = ((double)dst_pos[0].col) / (rotate_w - 1);
                normal_y = ((double)dst_pos[0].row) / (rotate_h - 1)*(-1);
                scale_pos[0].col = (((normal_x - 0.5) * cosRot + (normal_y + 0.5) * (-sinRot)) + 0.5)*(scale_w - 1);
                scale_pos[0].row = (((normal_y + 0.5) * cosRot + (normal_x - 0.5) * sinRot) - 0.5)*(scale_h - 1)*(-1);
                normal_x = ((double)dst_pos[1].col) / (rotate_w - 1);
                normal_y = ((double)dst_pos[1].row) / (rotate_h - 1)*(-1);
                scale_pos[1].col = (((normal_x - 0.5) * cosRot + (normal_y + 0.5) * (-sinRot)) + 0.5)*(scale_w - 1);
                scale_pos[1].row = (((normal_y + 0.5) * cosRot + (normal_x - 0.5) * sinRot) - 0.5)*(scale_h - 1)*(-1);
                normal_x = ((double)dst_pos[2].col) / (rotate_w - 1);
                normal_y = ((double)dst_pos[2].row) / (rotate_h - 1)*(-1);
                scale_pos[2].col = (((normal_x - 0.5) * cosRot + (normal_y + 0.5) * (-sinRot)) + 0.5)*(scale_w - 1);
                scale_pos[2].row = (((normal_y + 0.5) * cosRot + (normal_x - 0.5) * sinRot) - 0.5)*(scale_h - 1)*(-1);
                normal_x = ((double)dst_pos[3].col) / (rotate_w - 1);
                normal_y = ((double)dst_pos[3].row) / (rotate_h - 1)*(-1);
                scale_pos[3].col = (((normal_x - 0.5) * cosRot + (normal_y + 0.5) * (-sinRot)) + 0.5)*(scale_w - 1);
                scale_pos[3].row = (((normal_y + 0.5) * cosRot + (normal_x - 0.5) * sinRot) - 0.5)*(scale_h - 1)*(-1);
            }
            else
            {
                scale_pos[0].col = dst_pos[0].col;
                scale_pos[0].row = dst_pos[0].row;
                scale_pos[1].col = dst_pos[1].col;
                scale_pos[1].row = dst_pos[1].row;
                scale_pos[2].col = dst_pos[2].col;
                scale_pos[2].row = dst_pos[2].row;
                scale_pos[3].col = dst_pos[3].col;
                scale_pos[3].row = dst_pos[3].row;
            }

            if (trans4_para->scale_enable == 1)
            {
                crop_pos[0].col = scale_pos[0].col*scale_r_w;
                crop_pos[0].row = scale_pos[0].row*scale_r_h;
                crop_pos[1].col = scale_pos[1].col*scale_r_w;
                crop_pos[1].row = scale_pos[1].row*scale_r_h;
                crop_pos[2].col = scale_pos[2].col*scale_r_w;
                crop_pos[2].row = scale_pos[2].row*scale_r_h;
                crop_pos[3].col = scale_pos[3].col*scale_r_w;
                crop_pos[3].row = scale_pos[3].row*scale_r_h;
            }
            else
            {
                crop_pos[0].col = scale_pos[0].col;
                crop_pos[0].row = scale_pos[0].row;
                crop_pos[1].col = scale_pos[1].col;
                crop_pos[1].row = scale_pos[1].row;
                crop_pos[2].col = scale_pos[2].col;
                crop_pos[2].row = scale_pos[2].row;
                crop_pos[3].col = scale_pos[3].col;
                crop_pos[3].row = scale_pos[3].row;
            }

            if (trans4_para->crop_enable == 1)
            {
                trans4_pos[0].col = crop_pos[0].col + crop_x;
                trans4_pos[0].row = crop_pos[0].row + crop_y;
                trans4_pos[1].col = crop_pos[1].col + crop_x;
                trans4_pos[1].row = crop_pos[1].row + crop_y;
                trans4_pos[2].col = crop_pos[2].col + crop_x;
                trans4_pos[2].row = crop_pos[2].row + crop_y;
                trans4_pos[3].col = crop_pos[3].col + crop_x;
                trans4_pos[3].row = crop_pos[3].row + crop_y;
            }
            else
            {
                trans4_pos[0].col = crop_pos[0].col;
                trans4_pos[0].row = crop_pos[0].row;
                trans4_pos[1].col = crop_pos[1].col;
                trans4_pos[1].row = crop_pos[1].row;
                trans4_pos[2].col = crop_pos[2].col;
                trans4_pos[2].row = crop_pos[2].row;
                trans4_pos[3].col = crop_pos[3].col;
                trans4_pos[3].row = crop_pos[3].row;
            }

            switch (trans4_para->dist_function)
            {
            case(DISTRECT_TRAN4_NEW) :
            {
                src_pos[0] = get_trans4_new_coord(trans4_para->trans4_factor, trans4_pos + 0);
                src_pos[1] = get_trans4_new_coord(trans4_para->trans4_factor, trans4_pos + 1);
                src_pos[2] = get_trans4_new_coord(trans4_para->trans4_factor, trans4_pos + 2);
                src_pos[3] = get_trans4_new_coord(trans4_para->trans4_factor, trans4_pos + 3);

                break;
            }
            default:
            {
                src_pos[0] = trans4_pos[0];
                src_pos[1] = trans4_pos[1];
                src_pos[2] = trans4_pos[2];
                src_pos[3] = trans4_pos[3];
                break;
            }
            }
            weitStep.col_step1 = (src_pos[0].col);
            weitStep.col_step2 = (pcmd[block_cnt].out_res.w == 1) ? 0 : (src_pos[1].col - src_pos[0].col) / (pcmd[block_cnt].out_res.w - 1);
            weitStep.col_step3 = (pcmd[block_cnt].out_res.h == 1) ? 0 : (src_pos[2].col - src_pos[0].col) / (pcmd[block_cnt].out_res.h - 1);
            weitStep.col_step4 = ((pcmd[block_cnt].out_res.w == 1) || (pcmd[block_cnt].out_res.h == 1)) ? 0 : (src_pos[0].col + src_pos[3].col - src_pos[1].col - src_pos[2].col) / ((pcmd[block_cnt].out_res.w - 1) * (pcmd[block_cnt].out_res.h - 1));
            weitStep.row_step1 = (src_pos[0].row);
            weitStep.row_step2 = (pcmd[block_cnt].out_res.w == 1) ? 0 : (src_pos[1].row - src_pos[0].row) / (pcmd[block_cnt].out_res.w - 1);
            weitStep.row_step3 = (pcmd[block_cnt].out_res.h == 1) ? 0 : (src_pos[2].row - src_pos[0].row) / (pcmd[block_cnt].out_res.h - 1);
            weitStep.row_step4 = ((pcmd[block_cnt].out_res.w == 1) || (pcmd[block_cnt].out_res.h == 1)) ? 0 : (src_pos[0].row + src_pos[3].row - src_pos[1].row - src_pos[2].row) / ((pcmd[block_cnt].out_res.w - 1) * (pcmd[block_cnt].out_res.h - 1));

            pcmd[block_cnt].weit_step.col_step1 = quantify_s32(weitStep.col_step1, 4);
            pcmd[block_cnt].weit_step.col_step2 = quantify_s16(weitStep.col_step2, 10);
            pcmd[block_cnt].weit_step.col_step3 = quantify_s16(weitStep.col_step3, 10);
            pcmd[block_cnt].weit_step.col_step4 = quantify_s32(weitStep.col_step4, 16);
            pcmd[block_cnt].weit_step.row_step1 = quantify_s32(weitStep.row_step1, 4);
            pcmd[block_cnt].weit_step.row_step2 = quantify_s16(weitStep.row_step2, 10);
            pcmd[block_cnt].weit_step.row_step3 = quantify_s16(weitStep.row_step3, 10);
            pcmd[block_cnt].weit_step.row_step4 = quantify_s32(weitStep.row_step4, 16);

            block_cnt = block_cnt + 1;
        }
    }
    trans4_para->block_cnt = block_cnt;
}


T_COORD_DOUBLE get_trans4_new_coord(
    T_LEAST_TRANS4_NEW trans4_factor, T_COORD_DOUBLE* std_pos)
{
    double PI = 3.14159265358979323846;
    double f_length = (double)trans4_factor.f_length_pixel;
    int width = trans4_factor.width;
    int height = trans4_factor.height;
    int direction = trans4_factor.cylindrical_direction;

    double camera_m1[3][3];
    double camera_m2[3][3];
    double dist_coeffs[8];
    double perspective_tran4_mat[3][3];
    memcpy(camera_m1, trans4_factor.camera_m1, 3 * 3 * sizeof(double));
    memcpy(camera_m2, trans4_factor.camera_m2, 3 * 3 * sizeof(double));
    memcpy(dist_coeffs, trans4_factor.dist_coeffs, 8 * sizeof(double));
    memcpy(perspective_tran4_mat, trans4_factor.perspective_tran4_mat, 3 * 3 * sizeof(double));

    T_COORD_DOUBLE temp_pos;
    T_COORD_DOUBLE warped_pos;
    T_COORD_DOUBLE norm_pos;
    T_COORD_DOUBLE trans4_pos;
    T_COORD_DOUBLE cylindrical_pos, central_pos;
    double r2, r4, r6, rad1, rad2, tan1, tan2, warped_w, theta, h;


    temp_pos.col = std_pos->col;
    temp_pos.row = std_pos->row;

    if (trans4_factor.perspective_transform_enable == 1)
    {
        warped_pos.col = perspective_tran4_mat[0][0] * temp_pos.col + perspective_tran4_mat[0][1] * temp_pos.row + perspective_tran4_mat[0][2];
        warped_pos.row = perspective_tran4_mat[1][0] * temp_pos.col + perspective_tran4_mat[1][1] * temp_pos.row + perspective_tran4_mat[1][2];
        warped_w = perspective_tran4_mat[2][0] * temp_pos.col + perspective_tran4_mat[2][1] * temp_pos.row + perspective_tran4_mat[2][2];

        if (warped_w != 0)
        {
            warped_pos.col = warped_pos.col / warped_w;
            warped_pos.row = warped_pos.row / warped_w;
        }
        else
        {
            warped_pos.col = -1;
            warped_pos.row = -1;
        }

        temp_pos.col = warped_pos.col;
        temp_pos.row = warped_pos.row;
    }

    if (trans4_factor.camera_calibrate_enable == 1)
    {

        norm_pos.col = (temp_pos.col - camera_m2[0][2]) / camera_m2[0][0];
        norm_pos.row = (temp_pos.row - camera_m2[1][2]) / camera_m2[1][1];

        r2 = norm_pos.col * norm_pos.col + norm_pos.row * norm_pos.row;
        r4 = r2 * r2;
        r6 = r2 * r2 * r2;

        rad1 = 1.0 + dist_coeffs[0] * r2 + dist_coeffs[1] * r4 + dist_coeffs[4] * r6;
        rad2 = 1.0 + dist_coeffs[5] * r2 + dist_coeffs[6] * r4 + dist_coeffs[7] * r6;

        tan1 = 2.0 * dist_coeffs[2] * norm_pos.col * norm_pos.row + dist_coeffs[3] * (r2 + 2 * norm_pos.col * norm_pos.col);
        tan2 = dist_coeffs[2] * (r2 + 2 * norm_pos.row * norm_pos.row) + 2.0 * dist_coeffs[3] * norm_pos.col * norm_pos.row;

        temp_pos.col = (norm_pos.col * rad1 / rad2 + tan1) * camera_m1[0][0] + camera_m1[0][2];
        temp_pos.row = (norm_pos.row * rad1 / rad2 + tan2) * camera_m1[1][1] + camera_m1[1][2];
    }

    trans4_pos.col = temp_pos.col;
    trans4_pos.row = temp_pos.row;

    return trans4_pos;
}

int32_t quantify_s32(double x, uint8_t bit)
{
    uint32_t y;
    y = 1 << bit;
    return (int32_t)((x >= 0) ? (x * y + 0.5) : (x * y - 0.5));
}

int16_t quantify_s16(double x, uint8_t bit)
{
    uint32_t y;
    y = 1 << bit;
    return (int16_t)((x >= 0) ? (x * y + 0.5) : (x * y - 0.5));
}

#ifdef __cplusplus
}
#endif
