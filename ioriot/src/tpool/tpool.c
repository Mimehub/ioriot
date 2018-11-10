// Copyright 2018 Mimecast Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "tpool.h"

static void* _tpool_start_thread(void *t_)
{
    tpool_s *t = t_;

    Loop {
        pthread_mutex_lock(&t->mutex);

        while (!rbuffer_has_next(t->datas))
        {
            if (t->terminate) {
                pthread_mutex_unlock(&t->mutex);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&t->not_empty, &t->mutex);
        }

        void *data = rbuffer_get_next(t->datas);
        void *data2 = rbuffer_get_next(t->datas2);
        void *data3 = rbuffer_get_next(t->datas3);

        pthread_cond_signal(&t->not_full);
        pthread_mutex_unlock(&t->mutex);

        t->callback(data, data2, data3);
    }
}

tpool_s *tpool_new(int num_threads, void (*callback)(void *data, void *data2, void *data3))
{
    tpool_s *t = Malloc(tpool_s);

    pthread_mutex_init(&t->mutex, NULL);
    pthread_cond_init(&t->not_full, NULL);
    pthread_cond_init(&t->not_empty, NULL);

    t->num_threads = num_threads;
    t->threads = Calloc(num_threads, pthread_t);
    t->callback = callback;
    t->terminate = false;

    t->datas = rbuffer_new(num_threads*2);
    t->datas2 = rbuffer_new(num_threads*2);
    t->datas3 = rbuffer_new(num_threads*2);

    for (int i = 0; i < t->num_threads; ++i)
        start_pthread(&t->threads[i], _tpool_start_thread, t);

    return t;
}

void tpool_destroy(tpool_s *t)
{
    pthread_mutex_lock(&t->mutex);
    t->terminate = true;
    pthread_cond_broadcast(&t->not_empty);
    pthread_mutex_unlock(&t->mutex);

    tpool_join_threads(t);
    pthread_mutex_destroy(&t->mutex);
    pthread_cond_destroy(&t->not_full);
    pthread_cond_destroy(&t->not_empty);

    rbuffer_destroy(t->datas);
    rbuffer_destroy(t->datas2);
    rbuffer_destroy(t->datas3);

    free(t->threads);
    free(t);
}

void tpool_join_threads(tpool_s *t)
{
    for (int i = 0; i < t->num_threads; ++i)
        pthread_join(t->threads[i], NULL);
}

void tpool_add_work3(tpool_s* t, void *data, void *data2, void *data3)
{
    pthread_mutex_lock(&t->mutex);

    while (!rbuffer_insert(t->datas, (void*)data))
        pthread_cond_wait(&t->not_full, &t->mutex);
    rbuffer_insert(t->datas2, data2);
    rbuffer_insert(t->datas3, data3);

    pthread_cond_signal(&t->not_empty);
    pthread_mutex_unlock(&t->mutex);
}
static void _tpool_test_str_callback(void *data, void *data2, void *data3)
{
    Put("tpool_callback data:%s data2:%s data3:%s",
        (char*)data, (char*)data2, (char*)data3);
}

unsigned long _tpool_test_sum = 0;
pthread_mutex_t _tpool_test_mutex;

static void _tpool_test_long_callback(void *data, void *data2, void *data3)
{
    pthread_mutex_lock(&_tpool_test_mutex);
    _tpool_test_sum += (long)data + (long)data2 + (long)data3;
    pthread_mutex_unlock(&_tpool_test_mutex);
}

void tpool_test(void)
{
    pthread_mutex_init(&_tpool_test_mutex, NULL);

    tpool_s* t = tpool_new(12, _tpool_test_str_callback);
    tpool_add_work3(t, "eat", "my", "donut");
    tpool_destroy(t);

    t = tpool_new(10, _tpool_test_long_callback);

    unsigned long sum = 0;
    for (long i = 0; i < 100; ++i)
        tpool_add_work3(t, (void*)(i), (void*)(2*i), (void*)(i+2*i));
    for (long i = 0; i < 100; ++i)
        sum += i + 2*i + i+2*i;

    tpool_destroy(t);

    assert(sum == _tpool_test_sum);
    pthread_mutex_destroy(&_tpool_test_mutex);
}
