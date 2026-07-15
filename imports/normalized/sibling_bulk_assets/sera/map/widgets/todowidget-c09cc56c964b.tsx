/**
 * Todo Widget
 * 
 * Tasks and reminders management.
 */

'use client';

import React, { useState, useCallback } from 'react';
import { clsx } from 'clsx';
import { format, isToday, isTomorrow, isPast, parseISO } from 'date-fns';
import {
  CheckCircle2,
  Circle,
  Plus,
  Calendar,
  Flag,
  MoreHorizontal,
  Trash2,
  Edit3,
  X,
} from 'lucide-react';
import type { WidgetProps, TodoWidgetConfig, TodoItem } from '../types';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { ScrollArea } from '@/components/ui/scroll-area';
import { Input } from '@/components/ui/input';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';

// Priority configuration
const priorityConfig = {
  high: { color: 'text-red-500', bg: 'bg-red-50', label: 'High' },
  medium: { color: 'text-yellow-500', bg: 'bg-yellow-50', label: 'Medium' },
  low: { color: 'text-blue-500', bg: 'bg-blue-50', label: 'Low' },
};

// Mock initial data
const mockTodos: TodoItem[] = [
  {
    id: '1',
    title: 'Complete incident report',
    description: 'Document the afternoon incident',
    completed: false,
    priority: 'high',
    dueDate: new Date().toISOString(),
    category: 'Documentation',
    createdAt: new Date(Date.now() - 86400000).toISOString(),
  },
  {
    id: '2',
    title: 'Medication count',
    description: 'End of shift medication inventory',
    completed: false,
    priority: 'high',
    dueDate: new Date(Date.now() + 3600000).toISOString(),
    category: 'Medication',
    createdAt: new Date(Date.now() - 7200000).toISOString(),
  },
  {
    id: '3',
    title: 'Review care plan updates',
    description: 'Check for any new care plan modifications',
    completed: false,
    priority: 'medium',
    dueDate: new Date(Date.now() + 86400000).toISOString(),
    category: 'Care',
    createdAt: new Date(Date.now() - 43200000).toISOString(),
  },
  {
    id: '4',
    title: 'Submit timesheet',
    description: 'Weekly hours verification',
    completed: true,
    priority: 'low',
    dueDate: new Date(Date.now() - 86400000).toISOString(),
    category: 'Admin',
    createdAt: new Date(Date.now() - 172800000).toISOString(),
  },
];

// Format due date
function formatDueDate(dateString?: string): { text: string; isOverdue: boolean; isToday: boolean } {
  if (!dateString) return { text: 'No due date', isOverdue: false, isToday: false };
  
  const date = parseISO(dateString);
  if (isToday(date)) return { text: 'Today', isOverdue: isPast(date) && !isToday(date), isToday: true };
  if (isTomorrow(date)) return { text: 'Tomorrow', isOverdue: false, isToday: false };
  if (isPast(date)) return { text: format(date, 'MMM d'), isOverdue: true, isToday: false };
  return { text: format(date, 'MMM d'), isOverdue: false, isToday: false };
}

export function TodoWidget({ config, className }: WidgetProps) {
  const todoConfig = config as TodoWidgetConfig;
  const [todos, setTodos] = useState<TodoItem[]>(mockTodos);
  const [newTodoTitle, setNewTodoTitle] = useState('');
  const [isAdding, setIsAdding] = useState(false);
  const [editingId, setEditingId] = useState<string | null>(null);

  // Filter and sort todos
  const filteredTodos = todos
    .filter((todo) => todoConfig.showCompleted || !todo.completed)
    .sort((a, b) => {
      if (a.completed !== b.completed) return a.completed ? 1 : -1;
      
      switch (todoConfig.sortBy || 'dueDate') {
        case 'dueDate':
          if (!a.dueDate) return 1;
          if (!b.dueDate) return -1;
          return new Date(a.dueDate).getTime() - new Date(b.dueDate).getTime();
        case 'priority':
          const priorityOrder = { high: 0, medium: 1, low: 2 };
          return priorityOrder[a.priority] - priorityOrder[b.priority];
        case 'created':
        default:
          return new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime();
      }
    })
    .slice(0, todoConfig.limit || 10);

  const toggleTodo = useCallback((id: string) => {
    setTodos((prev) =>
      prev.map((todo) =>
        todo.id === id ? { ...todo, completed: !todo.completed } : todo
      )
    );
  }, []);

  const deleteTodo = useCallback((id: string) => {
    setTodos((prev) => prev.filter((todo) => todo.id !== id));
  }, []);

  const addTodo = useCallback(() => {
    if (!newTodoTitle.trim()) return;

    const newTodo: TodoItem = {
      id: Date.now().toString(),
      title: newTodoTitle.trim(),
      completed: false,
      priority: 'medium',
      createdAt: new Date().toISOString(),
    };

    setTodos((prev) => [newTodo, ...prev]);
    setNewTodoTitle('');
    setIsAdding(false);
  }, [newTodoTitle]);

  const updateTodo = useCallback((id: string, updates: Partial<TodoItem>) => {
    setTodos((prev) =>
      prev.map((todo) => (todo.id === id ? { ...todo, ...updates } : todo))
    );
  }, []);

  const completedCount = todos.filter((t) => t.completed).length;
  const totalCount = todos.length;

  return (
    <Card className={clsx('h-full flex flex-col', className)}>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between">
          <CardTitle className="text-lg font-semibold">
            {config.title || 'My Tasks'}
          </CardTitle>
          <Badge variant="secondary" className="text-xs">
            {completedCount}/{totalCount}
          </Badge>
        </div>
      </CardHeader>

      <CardContent className="flex-1 p-0 flex flex-col">
        {/* Add new todo */}
        {isAdding ? (
          <div className="px-4 pb-3">
            <div className="flex gap-2">
              <Input
                value={newTodoTitle}
                onChange={(e) => setNewTodoTitle(e.target.value)}
                placeholder="Add a new task..."
                className="flex-1"
                autoFocus
                onKeyDown={(e) => {
                  if (e.key === 'Enter') addTodo();
                  if (e.key === 'Escape') {
                    setIsAdding(false);
                    setNewTodoTitle('');
                  }
                }}
              />
              <Button size="icon" variant="ghost" onClick={addTodo}>
                <CheckCircle2 className="h-4 w-4" />
              </Button>
              <Button
                size="icon"
                variant="ghost"
                onClick={() => {
                  setIsAdding(false);
                  setNewTodoTitle('');
                }}
              >
                <X className="h-4 w-4" />
              </Button>
            </div>
          </div>
        ) : (
          <div className="px-4 pb-3">
            <Button
              variant="outline"
              className="w-full justify-start"
              onClick={() => setIsAdding(true)}
            >
              <Plus className="h-4 w-4 mr-2" />
              Add task
            </Button>
          </div>
        )}

        {/* Todo list */}
        <ScrollArea className="flex-1 px-4">
          <div className="space-y-2 pb-4">
            {filteredTodos.length === 0 ? (
              <div className="text-center py-8 text-gray-400">
                <CheckCircle2 className="h-8 w-8 mx-auto mb-2 opacity-50" />
                <p className="text-sm">No tasks</p>
              </div>
            ) : (
              filteredTodos.map((todo) => (
                <TodoItemComponent
                  key={todo.id}
                  todo={todo}
                  isEditing={editingId === todo.id}
                  onToggle={() => toggleTodo(todo.id)}
                  onDelete={() => deleteTodo(todo.id)}
                  onEdit={() => setEditingId(todo.id)}
                  onUpdate={(updates) => {
                    updateTodo(todo.id, updates);
                    setEditingId(null);
                  }}
                  onCancelEdit={() => setEditingId(null)}
                />
              ))
            )}
          </div>
        </ScrollArea>
      </CardContent>
    </Card>
  );
}

// Individual todo item
interface TodoItemComponentProps {
  todo: TodoItem;
  isEditing: boolean;
  onToggle: () => void;
  onDelete: () => void;
  onEdit: () => void;
  onUpdate: (updates: Partial<TodoItem>) => void;
  onCancelEdit: () => void;
}

function TodoItemComponent({
  todo,
  isEditing,
  onToggle,
  onDelete,
  onEdit,
  onUpdate,
  onCancelEdit,
}: TodoItemComponentProps) {
  const [editTitle, setEditTitle] = useState(todo.title);
  const dueInfo = formatDueDate(todo.dueDate);
  const priority = priorityConfig[todo.priority];

  if (isEditing) {
    return (
      <div className="flex gap-2 p-2 bg-gray-50 rounded-lg">
        <Input
          value={editTitle}
          onChange={(e) => setEditTitle(e.target.value)}
          className="flex-1"
          autoFocus
          onKeyDown={(e) => {
            if (e.key === 'Enter') onUpdate({ title: editTitle });
            if (e.key === 'Escape') onCancelEdit();
          }}
        />
        <Button
          size="icon"
          variant="ghost"
          onClick={() => onUpdate({ title: editTitle })}
        >
          <CheckCircle2 className="h-4 w-4" />
        </Button>
      </div>
    );
  }

  return (
    <div
      className={clsx(
        'group flex items-start gap-2 p-2 rounded-lg transition-colors',
        'hover:bg-gray-50',
        todo.completed && 'opacity-50'
      )}
    >
      <button
        onClick={onToggle}
        className={clsx(
          'mt-0.5 transition-colors',
          todo.completed ? 'text-green-500' : 'text-gray-400 hover:text-gray-600'
        )}
      >
        {todo.completed ? (
          <CheckCircle2 className="h-5 w-5" />
        ) : (
          <Circle className="h-5 w-5" />
        )}
      </button>

      <div className="flex-1 min-w-0">
        <p
          className={clsx(
            'text-sm font-medium truncate',
            todo.completed && 'line-through text-gray-400'
          )}
        >
          {todo.title}
        </p>

        <div className="flex items-center gap-2 mt-1">
          {todo.dueDate && (
            <span
              className={clsx(
                'text-xs flex items-center gap-1',
                dueInfo.isOverdue ? 'text-red-500' : 'text-gray-400'
              )}
            >
              <Calendar className="h-3 w-3" />
              {dueInfo.text}
            </span>
          )}

          <Badge
            variant="ghost"
            className={clsx('text-[10px] px-1.5 py-0', priority.bg, priority.color)}
          >
            <Flag className="h-2.5 w-2.5 mr-1" />
            {priority.label}
          </Badge>

          {todo.category && (
            <span className="text-xs text-gray-400">{todo.category}</span>
          )}
        </div>
      </div>

      <DropdownMenu>
        <DropdownMenuTrigger asChild>
          <Button
            variant="ghost"
            size="icon"
            className="h-7 w-7 opacity-0 group-hover:opacity-100 transition-opacity"
          >
            <MoreHorizontal className="h-3.5 w-3.5" />
          </Button>
        </DropdownMenuTrigger>
        <DropdownMenuContent align="end">
          <DropdownMenuItem onClick={onEdit}>
            <Edit3 className="h-4 w-4 mr-2" />
            Edit
          </DropdownMenuItem>
          <DropdownMenuItem onClick={onDelete} className="text-red-500">
            <Trash2 className="h-4 w-4 mr-2" />
            Delete
          </DropdownMenuItem>
        </DropdownMenuContent>
      </DropdownMenu>
    </div>
  );
}

export default TodoWidget;
